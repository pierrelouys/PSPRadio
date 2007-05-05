#include "irc.h"
#include "dlib/util.h"
#include <netdb.h>

#include <iostream>

///Some internal Functions

//extract nick from a :bob!something@something string, if not valid return whole string
string getNickFromInfo(const string &info)
{
	int startPos = 0;
	if(info[0] == ':')
		startPos++;
	
	string::size_type exclamation = info.find("!", startPos);
	if (exclamation == string::npos)
		return info;
	else
		return info.substr(startPos, exclamation-startPos);
}

///ircPerson
ircPerson::ircPerson() {} //DONT USE
ircPerson::ircPerson(string joinInfo) // nick!user@host
{
	//set defaults
	voice = false;
	op = false;
	hop = false;
	realname = "";
	knowrealname = false;
	
	//Extract the details from the string
	if (joinInfo[0] == ':') joinInfo = joinInfo.substr(1);
	int exPos = joinInfo.find("!");
	nick = joinInfo.substr(0, exPos);
	int atPos = joinInfo.find("@", exPos);
	user = joinInfo.substr(exPos+1, atPos-exPos-1);
	host = joinInfo.substr(atPos+1, joinInfo.length());
}

ircPerson::ircPerson(const string &nNick, const string &nRealname, const string &nUser, const string &nHost, const string &nMode)
{
	nick = nNick;
	realname = nRealname;
	user     = nUser;
	host     = nHost;
	
	if (nMode.find("+") != string::npos) voice = true; else voice = false;
	if (nMode.find("@") != string::npos) op    = true; else op    = false;
	if (nMode.find("%") != string::npos) hop   = true; else hop   = false;
}

///channelDetails

//DON'T USE, required so channelDetails can be used properly with a map
channelDetails::channelDetails()
{
	channelName = "INVALIDCHANNEL";
}
 
channelDetails::channelDetails(const string &name, const string &topic)
{
	channelName = name;
	channelTopic = topic;
}

void channelDetails::addPerson(const ircPerson& person)
{
	//Insert in order
	list<ircPerson>::iterator iter;
	for (iter=channelPeople.begin(); iter != channelPeople.end(); iter++)
	{
		if ((*iter).nick > person.nick)
		{
			channelPeople.insert(iter, person);
			return;
		}
	}
	
	channelPeople.push_back(person);
}

void channelDetails::renamePerson(const string &oldNick, const string &newNick)
{
	list<ircPerson>::iterator iter;
	for (iter=channelPeople.begin(); iter != channelPeople.end(); iter++)
	{
		if ((*iter).nick == oldNick)
		{
			(*iter).nick = newNick;
			break;
		}
	}
}

void channelDetails::removePerson(const string& nick)
{
	list<ircPerson>::iterator iter;
	for (iter=channelPeople.begin(); iter != channelPeople.end(); iter++)
	{
		if ((*iter).nick == nick)
		{
			channelPeople.erase(iter);
			break;
		}
	}
}


//used by irc class when receiving a MODE message.
//parameters will be of format: 
//source - ":bob!aaa@bbb" (use getNickFromInfo) - person/server that did change
//modeString - "+vv Randy DuEy" or "+t" etc...
void channelDetails::changeMode(ircCallback* callback, const string source, const string modeString)
{
//TODO: doesn't handle many valid modes (topic?)

	vector < string > split = explode(modeString, ' ');
	int cur = 1; //index in split list currently applying to
	
	bool inPlus = true; //needs a default, who cares
	
	for (unsigned int a = 0; a < split[0].size(); a++)
	{
		switch(split[0][a])
		{
		case '+':
			inPlus = true;
			break;
		case '-':
			inPlus = false;
			break;
		
		case 'o':
			getPerson(split[cur]).op = inPlus;
			if (inPlus)	callback->channelChangeCallback(channelName, CC_OP, split[cur]);
			else		callback->channelChangeCallback(channelName, CC_DEOP, split[cur]);
			cur++;
			break;
			
		case 'v':
			getPerson(split[cur]).voice = inPlus;
			if (inPlus)	callback->channelChangeCallback(channelName, CC_VOICE, split[cur]);
			else		callback->channelChangeCallback(channelName, CC_DEVOICE, split[cur]);
			cur++;
			break;
			
		case 'h':
			getPerson(split[cur]).hop = inPlus;
			if (inPlus)	callback->channelChangeCallback(channelName, CC_HOP, split[cur]);
			else		callback->channelChangeCallback(channelName, CC_DEHOP, split[cur]);
			cur++;
			break;
		
		}
	}
}

ircPerson& channelDetails::getPerson(const string& nick)
{
	list<ircPerson>::iterator iter;
	for (iter = channelPeople.begin(); iter != channelPeople.end(); iter++)
	{
		if ((*iter).nick == nick)
			return *iter;
	}

	//THIS SHOULD NEVER HAPPEN
}

#ifdef PSP
///PSP Specific resolver hack.... uses threads so that we don't get stuck if resolver locks up.
const char* PSP_strtoresolv = NULL;
struct hostent * PSP_resolvd = NULL;

int resolvThread(SceSize args, void *argp)
{
	PSP_resolvd = gethostbyname(PSP_strtoresolv);
	return 0;
}

struct hostent * PSP_gethostbyname(const char* addr, ircCallback* callback)
{
	PSP_strtoresolv = addr;
	PSP_resolvd = NULL;
	
	for (int a = 0; a < 4 && PSP_resolvd==NULL; a++) //try up to 4 times
	{
		SceUID dlthread = sceKernelCreateThread("afkim_resolver", resolvThread, 0x18, 0x10000, 0, NULL);
		sceKernelStartThread(dlthread, 0, NULL);
		
		unsigned int startTime = sceKernelGetSystemTimeLow();
		do
		{
			sceKernelDelayThread(100*1000);
		} while (PSP_resolvd == NULL && startTime+2000*1000 >= sceKernelGetSystemTimeLow()); //not resolved, within 2 seconds
		
		if (PSP_resolvd == NULL)
		{
			callback->serverCallback(SM_IRC_DETAILS, "Timeout while resolving");
		}
		int ret = sceKernelTerminateDeleteThread(dlthread);
		if (ret < 0)
		{
			cout << "Failed to kill downloading thread, ignoring. (this will likely cause problems)\n";
		}
		else
		{
			cout << "Thread killed\n";
		}
	}
	
	return PSP_resolvd;
}
#endif


///irc
irc::irc(ircCallback* CB)
{
	callback = CB;
	CB->myirc = this;
	
	sock = 0;
	
	status = CS_OFFLINE;
}

bool irc::doConnect(const string &nServer, const int &nPort, const string &nNick)
{
	//Disconnect if connected.
	if (status != CS_OFFLINE)
		doDisconnect();
	
	server  = nServer;
	port    = nPort;
	mNickname = nNick;
	
	
	status = CS_CONNECTING;
		
		//resolve
	struct hostent * resolvd;
	callback->serverCallback(SM_IRC_DETAILS, "Resolving");
	#ifdef PSP
	resolvd = PSP_gethostbyname(server.c_str(), callback);
	#else
	resolvd = gethostbyname(server.c_str());
	#endif
	if (resolvd == NULL)
	{
		status = CS_OFFLINE;
		callback->serverCallback(SM_IRC_ERROR, "Failed to resolve");
		return 0;
	}
	callback->serverCallback(SM_IRC_DETAILS, "Resolved, Connecting");
	
		//attempt nonblocking connect
	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = ((struct in_addr *)(resolvd->h_addr))->s_addr;
	saddr.sin_port = htons(port);
	
	//Create socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		status = CS_OFFLINE;
		callback->serverCallback(SM_IRC_ERROR, "Socket Creation Error");
		return 0;
	}
	
	//TODO make not blocking / add timeout
	if (connect(sock,(struct sockaddr *)  &saddr, sizeof(saddr)) == -1) {
		status = CS_OFFLINE;
		callback->serverCallback(SM_IRC_ERROR, "Connect error");
		return 0;
	}
	
	status = CS_IDENTIFYING;
	callback->serverCallback(SM_IRC_DETAILS, "Connected, Identifying");
		
		//send login details (UNKN = client type)
	string loginDetails = "NICK " + mNickname + "\r\nUSER " + mNickname + " UNKN SERVERADDR :" + mNickname + "\r\n";
	sendData(loginDetails);
	
	//poll untill connected or failed.
	while(status == CS_CONNECTING || status == CS_IDENTIFYING)
	{
		poll();
		usleep(1000*100);
	}
	
	if (status != CS_CONNECTED)
		return false;
	
	return true;
}

void irc::doDisconnect()
{
	sendData("QUIT :bai\r\n");
	close(sock);
	sock = 0;
	status = CS_OFFLINE;
	channels.clear();
}

const int TMP_BUF_SIZE = 1000;
char tmpBuf[TMP_BUF_SIZE];

void irc::poll()
{
	//Don't poll sockets if we are offline.
	if (status == CS_OFFLINE)
		return;
	
	fd_set socks;
	FD_ZERO(&socks);
	FD_SET(sock,&socks);
	
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;	
	
	int socksToRead = select(sock+1, &socks, (fd_set *) 0, (fd_set *) 0, &timeout);
	
	if (socksToRead < 0)
	{
		callback->serverCallback(SM_IRC_ERROR, "poll() Error");
		return;
	}
	else if (socksToRead == 0)
	{
		return;
	}
	
	//Socket has something
	int received = recv(sock, tmpBuf, TMP_BUF_SIZE-1, 0);
	if (received == -1)
	{
		close(sock);
		status = CS_OFFLINE;
		channels.clear();
		callback->serverCallback(SM_IRC_ERROR, "recv() Error");
		return;
	}
	
	//add all the new chars to the buffer, parsing it when we hit line ends
	for (int a = 0; a < received; a++)
	{
		if (tmpBuf[a] == '\r' || tmpBuf[a] == '\n')
		{
			processLineBuf();
		}
		else
		{
			currentLineBuf += tmpBuf[a];
		}
	}
}

void irc::sendPM(const string &target, const string &message)
{
	sendData("PRIVMSG " + target + " :" + message +"\r\n");
}

void irc::sendAction(const string &target, const string &message)
{
	sendData("PRIVMSG " + target + " :\1ACTION " + message +"\1\r\n");
}

void irc::setAway(const string &message)
{
	sendData("AWAY :" + message + "\r\n");
}

void irc::setBack()
{
	sendData("AWAY\r\n");
}

void irc::joinChannel(const string &channel)
{
	sendData("JOIN " + channel + "\r\n");
}

//TODO: I SHOULD BE USING THIS EVERYWHERE, saves writing \r\n everywhere!
void irc::sendRaw(const string &message)
{
	sendData(message+"\r\n");
}

void irc::whoIs(const string nick)
{
	sendData("whois " + nick + "\r\n");
}

void irc::pingServer()
{
	sendData("PING LAG123123123\r\n");
}

//PRIVATE
void irc::sendData(const string &data)
{
	if (send(sock, data.c_str(), data.length(), 0) == -1)
	{
		callback->serverCallback(SM_IRC_ERROR, "Error sending data");
	}
}

void irc::processLineBuf()
{
	if (currentLineBuf == "")
		return;
	cout << "RAW: " << currentLineBuf << endl;
	//Try parse 
	if (!parseFirst())
	{
		if (!parseSecond())
		{
			if (!parseSecondString())
			{
				//Unable to parse, print raw
				callback->serverCallback(SM_IRC_UNK, currentLineBuf);
			}
		}
	}
	currentLineBuf = "";
}

//First and Seconds only contain that string
#define GET_FIRST();	string::size_type firstSpacePos = currentLineBuf.find(" "); \
						if (firstSpacePos == string::npos) return false; \
						string first = currentLineBuf.substr(0, firstSpacePos);

#define GET_SECOND();	string::size_type secondSpacePos = currentLineBuf.find(" ", firstSpacePos+1); \
						if (secondSpacePos == string::npos) return false; \
						string second = currentLineBuf.substr(firstSpacePos+1, secondSpacePos-firstSpacePos-1);

#define GET_THIRD(); 	string::size_type thirdSpacePos = currentLineBuf.find(" ", secondSpacePos+1); \
						if (thirdSpacePos == string::npos) return false; \
						string third = currentLineBuf.substr(secondSpacePos+1, thirdSpacePos-secondSpacePos-1); \
						if (third[0] == ':') third = third.substr(1, third.length());

#define GET_FOURTH(); 	string::size_type fourthSpacePos = currentLineBuf.find(" ", thirdSpacePos+1); \
						if (fourthSpacePos == string::npos) return false; \
						string fourth = currentLineBuf.substr(thirdSpacePos+1, fourthSpacePos-thirdSpacePos-1); \
						if (fourth[0] == ':') fourth = fourth.substr(1, fourth.length());

//Third and Fourth plus contain all strings after that space (fourth+ requres GET_THIRD called first)
#define GET_THIRD_PLUS();	string third = currentLineBuf.substr(secondSpacePos+1, currentLineBuf.length()); \
							if(third[0] == ':') third=third.substr(1, third.length());

#define GET_FOURTH_PLUS();	string fourth = currentLineBuf.substr(thirdSpacePos+1, currentLineBuf.length()); \
							if(fourth[0] == ':') fourth=fourth.substr(1, fourth.length());

bool irc::parseFirst()
{
	GET_FIRST();
	
	if (first == "PING") //PING
	{
		currentLineBuf[1] = 'O';
		sendData(currentLineBuf+"\r\n");
		callback->serverCallback(SM_GENERAL_TEXT, "Ping? Pong!");
		return true;
	}
	
	if (first == "NOTICE") //NOTICE : this server is liek leet pwnz0r!
	{
		callback->serverCallback(SM_GENERAL_TEXT, currentLineBuf);
		return true;
	}
	
	return false;
}

bool irc::parseSecond()
{
	GET_FIRST();
	GET_SECOND();
	
	int secNum = atoi(second.c_str());
	if (secNum != 0) //It is a number
	{
		switch(secNum)
		{
		//Things we dont really care about
		
		///Things we chose to ignore completely
		case 333: //Someoneerather set the topic, TODO ACTUALLY USE THIS
		case 353: //people in a /names list
		case 366: //end of /names list
		case 221: // user mode is... (:scraps.workgroup 221 danzel +s)
			return true;
		///Straight message from the server to pass on.
		case 1:
		case 2:
		case 3:
		case 4://ServerName ServerVersion something something something
		case 5://Settings such supported by the server (possibly useful?)
			if (status == CS_IDENTIFYING)//Any of these mean that we are now connnected
			{
				status = CS_CONNECTED;   //We are connected yay!
				return true;
			}
		///Things that dont really matter, but are server messages
		case 251://User Invisible Servers
		case 252://ops online
		case 253://unknown connections
		case 254://channels formed
		case 255://i have X clients and Y servers (local server)
		case 375://pre MOTD (Start of MOTD?)
		case 372://do XXXX to read the MOTD
		case 376://end of MOTD
		{
			GET_THIRD();
			GET_FOURTH_PLUS();
			callback->serverCallback(SM_GENERAL_TEXT, fourth);
			return true;
		}
		
		///Error messages
		case 404://cannot send to channel
		case 421://unknown command
		{
			GET_THIRD_PLUS();
			callback->serverCallback(SM_NONFATAL_ERROR, third);
			return true;
		}
		case 433: //Nickname in use
		{
			GET_THIRD();
			GET_FOURTH();
			callback->serverCallback(SM_NICK_TAKEN, fourth);
			return true;
		}
		
		///Things we need to do something about
		
		case 332: //Channel topic is...
		{
			//:Oslo2.NO.EU.undernet.org 332 ircbotest #hamlan :hamLan V | 20-21 Jan 2007 | http://register.hamlan.co.nz/ | http://imdb.com/title/tt0479884/ you will watch
			//:server 332 whodidit channel topicishere
			//Get the chan
			GET_THIRD();
			GET_FOURTH();
			
			//FIXME: GET_FIFTHPLUS
			string topic;
			if (currentLineBuf[fourthSpacePos+1] == ':')
				topic = currentLineBuf.substr(fourthSpacePos+2, currentLineBuf.length());
			else
				topic = currentLineBuf.substr(fourthSpacePos+1, currentLineBuf.length());
			
			if (inChannel(fourth))
			{
				channels[fourth].channelTopic = topic;
			}
			
			callback->channelChangeCallback(fourth, CC_TOPIC, third + " " + topic);
			return true;
		}
		case 352: //People in ... is ..... (WHO list)
		{
			//"<server> 352 YOURNICK <channel> <user> <host> <server> <nick> <H|G>[*][@|+] :<hopcount> <real name>"
			// :im.bitlbee.org 352 danzel &bitlbee danzel 222-152-204-160.jetstream.xtra.co.nz im.bitlbee.org danzel H :0 Unknown
			// :im.bitlbee.org 352 danzel &bitlbee root im.bitlbee.org im.bitlbee.org root H :0 User manager
			
			//  0              1   2       3         4        5         6              7      8              9          10
			// <server>        352 URNICK <channel> <user>   <host>    <server>       <nick> <H|G>[*][@|+] :<hopcount> <real name>
			// :im.bitlbee.org 352 danzel &bitlbee stigmannz sdm.co.nz im.bitlbee.org stigmannz G :0 Rick Blaine ...
			// :im.bitlbee.org 315 danzel &bitlbee :End of /WHO list.
			
			vector<string> exploded = explode(currentLineBuf, ' ');
			
			if (!inChannel(exploded[3])) //someone from a channel we aren't in, WTF?
				return true;
			
			//check that this person isnt already in the chan.
			channels[exploded[3]].removePerson(exploded[7]);
			
			channels[exploded[3]].addPerson(ircPerson(exploded[7], exploded[10], exploded[4], exploded[5], exploded[8]));
			return true;
		}
		case 315: //End of /who list
		{
			// :ny.undernet.org 315 YOURNICK #CHANNEL :End of /WHO list.
			GET_THIRD();
			GET_FOURTH();
			
			callback->serverCallback(SM_WHO_DONE, fourth);
			
			return true;
		}
		
		//A couple of whois responses
		case 311: //Users realname and ipaddress
		{
			//:scraps 311 danzel thorxxx thorxxx hotmail.com * :Damion
			GET_THIRD();
			GET_FOURTH();
			
			//FIXME: finding ':' is a bit naughty
			string realname = currentLineBuf.substr(currentLineBuf.find(":", thirdSpacePos)+1, currentLineBuf.length());
			
			//FIXME: Should also get their IP and send it tooo~
			callback->serverCallback(SM_WHOIS_REALNAME, fourth + " " + realname);
			
			return true;
		}
		case 312: //server the user is connected to
		{
			//:scraps 312 danzel thorxxx my@msn_email.com. :MSN network
			
			GET_THIRD();
			GET_FOURTH();
			
			//FIXME: finding ':' is a bit naughty
			string theirserver = currentLineBuf.substr(currentLineBuf.find(":", thirdSpacePos)+1, currentLineBuf.length());
			
			callback->serverCallback(SM_WHOIS_SERVER, fourth + " " + theirserver);
			return true;
		}
		case 301: //Away info
		{
			//:scraps 301 danzel thor98x :Away
			GET_THIRD();
			GET_FOURTH();
			
			//FIXME: GET_FIFTHPLUS
			string awaytxt;
			if (currentLineBuf[fourthSpacePos+1] == ':')
				awaytxt = currentLineBuf.substr(fourthSpacePos+2, currentLineBuf.length());
			else
				awaytxt = currentLineBuf.substr(fourthSpacePos+1, currentLineBuf.length());
			
			callback->serverCallback(SM_WHOIS_AWAY, fourth + " " + awaytxt);
			return true;
		}
		case 318: //End of whois
		{
			//:scraps 318 danzel thor98x :End of /WHOIS list
			GET_THIRD();
			GET_FOURTH();
			
			callback->serverCallback(SM_WHOIS_END, fourth);
			
			return true;
		}
		
		
		}
	}
	
	return false;
}

bool irc::parseSecondString()
{
	GET_FIRST();
	GET_SECOND();
	
	if (second == "NOTICE") //Notice Message
	{
		GET_THIRD_PLUS();
		callback->serverCallback(SM_NOTICE, third);
		return true;
	}
	else if (second == "NICK")// nick change message
	{
		// :<NICK>!<USERNAME>@<HOST> NICK <NEWNICK>
		GET_THIRD_PLUS(); //third = newnick
		string oldnick = getNickFromInfo(first);
		
		if (oldnick == mNickname) mNickname = third; //If its our nickname

		//Rename the person in the nick list
		map<string,channelDetails>::iterator iter;
		for (iter = channels.begin(); iter != channels.end(); iter++)
		{
			iter->second.renamePerson(oldnick, third);
		}
		callback->serverCallback(SM_RENAME, oldnick+" "+third);
		
		return true;
	}
	else if (second == "PART") //Part message
	{
		//:dapples!dsawerwaer PART :#hamlan
		
		GET_THIRD_PLUS(); //HACK
		
		//TODO: Remove this hack and use get_third get_fourth+, 3=chan, 4=reason (may not be one)
		string::size_type spacePos;
		if ((spacePos = third.find(' ')) != string::npos)
		{
			third = third.substr(0,spacePos);
		}
		
		if (!inChannel(third)) //we aren't in this channel, wtf?
			return true;
		
		//get usernick
		string partnick = getNickFromInfo(first);

		channels[third].removePerson(partnick);
		
		callback->channelChangeCallback(third, CC_PART, partnick);
		
		if (partnick == mNickname) //if we've left
		{
			channels.erase(channels.find(third)); //leave the channel
			callback->serverCallback(SM_LEFT_CHAN, third);
		}
		return true;
	}
	else if (second == "JOIN") //Join message
	{
		GET_THIRD_PLUS();
		
		if (third[0] == ':') third = third.substr(1, third.length());
		string joinnick = getNickFromInfo(first);
		
		//its us, get list of people in chan
		if (joinnick == mNickname) 
		{
			if (!inChannel(third))
				channels[third] = channelDetails(third, "NO TOPIC???");
			channels[third].addPerson(ircPerson(first)); //Add ourself
			
			sendData("WHO " + third + "\r\n");
			callback->serverCallback(SM_JOIN_CHAN, third);
			return true;
		}
		
		if (!inChannel(third)) //channel we don't care about
			return true;
		
		channels[third].removePerson(joinnick);
		channels[third].addPerson(ircPerson(first));
		callback->channelChangeCallback(third, CC_JOIN, joinnick);
		return true;
	}
	else if (second == "KICK") //Kick message
	{
		//:WiZ KICK #Finnish John :Speaking English
		
		GET_THIRD();
		GET_FOURTH_PLUS();//HACK as there may not be a reason.
		string::size_type spacePos;
		if ((spacePos = fourth.find(" ")) != string::npos)
			fourth = fourth.substr(0, spacePos);
		
		callback->channelChangeCallback(third, CC_KICK, first + " " + fourth); //FIXME
		
		if (fourth == mNickname) //if we've left
		{
			channels.erase(channels.find(third)); //leave the channel
			callback->serverCallback(SM_LEFT_CHAN, third);
		}
		
		return true;
	}
	else if (second == "QUIT") //Quit message
	{
		GET_THIRD_PLUS();
		
		//get usernick
		string quitnick = getNickFromInfo(first);

		//Rename the person in the nick list
		map<string,channelDetails>::iterator iter;
		for (iter = channels.begin(); iter != channels.end(); iter++)
		{
			iter->second.removePerson(quitnick);
		}
		
		callback->serverCallback(SM_QUIT, quitnick + " " + third);
		return true;
	}
	else if (second == "MODE") //Mode message
	{
		//TODO: "server sets mode +x danzel" ??
	
		//:danzel!aaa@bbb MODE #hamlan +vv Randy DuEy
		GET_THIRD();
		GET_FOURTH_PLUS();
		
		string modenick = getNickFromInfo(first);
		
		
		if (!inChannel(third))
			return true;
		
		channels[third].changeMode(callback, first, fourth);
		
		callback->channelModeCallback(third, modenick, currentLineBuf.substr(secondSpacePos+1));
		return true;
	}
	else if (second == "TOPIC") //Topic Change message
	{
		//:danzel!fff@fff TOPIC #hamlan :safisjdf saeaower sadflksdf
		GET_THIRD();
		GET_FOURTH_PLUS();
		
		string topicnick = getNickFromInfo(first);
		
		if (!inChannel(third)) //Topic for a channel I aren't in? wtf?
			return true;
		
		channels[third].channelTopic = fourth; //TODO: Function?
		
		callback->channelChangeCallback(third, CC_TOPIC, topicnick + " " + fourth);
		return true;
	}
	else if (second == "INVITE") //Invite message
	{
		GET_THIRD();
		GET_FOURTH_PLUS();
		
		//:Angel!wings@irc.org INVITE Wiz #Dust
		string invitenick = getNickFromInfo(first);
		
		callback->serverCallback(SM_INVITE, invitenick + " " + fourth);
		
		return true;
	}
	else if (second == "PRIVMSG") //Private message
	{
		GET_THIRD();
		GET_FOURTH_PLUS();
		//privateMsgCallback   (string who,             string message)
		//channelMsgCallback   (string who,             string message)
		
		//:DuEy!aaa@aaa PRIVMSG #hamlan :omfg wtf bix nood
		//:danzel!~danzel@Danzel.users.undernet.org PRIVMSG #hamlan :\001ACTION tests\001
		//:danzel!~danzel@Danzel.users.undernet.org PRIVMSG #hamlan :\001VERSION\001
		//^^ reply  NOTICE danzel :\001VERSION STRING\001
		//TODO VERSION/ACTION
		string sender = getNickFromInfo(first);
		
		if (third == mNickname)	//PM
		{
			callback->privateMsgCallback(sender, fourth);
		}
		else	//Chan
		{
			callback->channelMsgCallback(third, sender, fourth);
		}
		return true;
	}
	else if (second == "PONG") //pong reply, throw it away
	{
		return true;
	}
	
	return false;
}

///Functions for getting information
string irc::getMyNick() const
{
	return mNickname;
}

const map<string, channelDetails>& irc::getChannels() const
{
	return channels;
}

//This won't compile as channels[channel] isn't really const :(
//TODO: do this a smarter way
const channelDetails& irc::getChannelDetails(const string &channel) const
{
	map<string, channelDetails>::const_iterator iter;
	for (iter = channels.begin(); iter != channels.end(); iter++)
	{
		if (iter->first == channel)
		{
			return iter->second;
		}
	}
	
	//OH SHI- NOT ALL RETURN ONE OHES NOES
}

bool irc::inChannel(const string &channel) const
{
	return channels.count(channel) != 0;
}
