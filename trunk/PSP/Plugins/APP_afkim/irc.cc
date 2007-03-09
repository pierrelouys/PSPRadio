#include "dlib/dlib.h"
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

//	printf("________Added Person \"%s\", \"%s\", \"%s\"\n", nick.c_str();
}

ircPerson::ircPerson(const string &nNick, const string &nRealname, const string &nUser, const string &nHost, const string &nMode)
{
	nick     = nNick;
	realname = nRealname;
	user     = nUser;
	host     = nHost;
	
	if (nMode.find("+") != string::npos) voice = true; else voice = false;
	if (nMode.find("@") != string::npos) op    = true; else op    = false;
	if (nMode.find("%") != string::npos) hop   = true; else hop   = false;
}

///irc
irc::irc(const string &nServer, const int &nPort, const string &nChannel, const string &nNick, ircCallback* CB)
{
	server  = nServer;
	port    = nPort;
	channel = nChannel;
	nick    = nNick;
	
	callback = CB;
	CB->myirc = this;
	
	sock = 0;
	
	status = CS_OFFLINE;
}

bool irc::doConnect()
{
	status = CS_CONNECTING;
		
		//resolve
	struct hostent * resolvd;
	callback->serverCallback(SM_IRC_DETAILS, "Resolving");
	resolvd = gethostbyname(server.c_str());
	if (resolvd == NULL)
	{
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
		callback->serverCallback(SM_IRC_ERROR, "Socket Creation Error");
		return 0;
	}
	
	//TODO make not blocking / add timeout
	if (connect(sock,(struct sockaddr *)  &saddr, sizeof(saddr)) == -1) {
		callback->serverCallback(SM_IRC_ERROR, "Connect error");
		return 0;
	}
	
/*	int flag = 1;
	int result = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
	if (result < 0)
	{
		//ignore the error!
	}*/
	
	status = CS_IDENTIFYING;
	callback->serverCallback(SM_IRC_DETAILS, "Connected, Identifying");
		
		//send login details (UNKN = client type)
	string loginDetails = "NICK " + nick + "\r\nUSER " + nick + " UNKN SERVERADDR :" + nick + "\r\n";
	sendData(loginDetails);
	
	//TODO - what if we go into an error state in here!
	
	while(status != CS_CONNECTED && status != CS_INCHAN)
	{
		poll();
		usleep(1000*100);
	}
	
	//join channel
	sendData("JOIN " + channel + "\r\n");
	
	//wait for userlist
	while (status != CS_INCHAN)
	{
		poll();
		usleep(1000*100);
	}
	
	return true;
}

void irc::doDisconnect()
{
	sendData("QUIT :bai\r\n");
	close(sock);
	sock = 0;
	status = CS_OFFLINE;
}
//	list<ircperson> channelPeople;

const int TMP_BUF_SIZE = 1000;
char tmpBuf[TMP_BUF_SIZE];

void irc::poll()
{
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

void irc::setAway(const string &message)
{
	sendData("AWAY :" + message + "\r\n");
}

void irc::setBack()
{
	sendData("AWAY\r\n");
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
			//Get the chan
			GET_THIRD();
			GET_FOURTH_PLUS();
			
			callback->channelChangeCallback(CC_TOPIC, fourth);
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
			
			if (exploded[3] != channel) //someone from another channel
				return true;
			
				//loop to check that this person isnt already in the chan.
			channelUserLeave(exploded[7]);
			
			//TODO add sorted  -- insert(I,T)	Insert an element before I.
			channelPeople.push_back(ircPerson(exploded[7], exploded[10], exploded[4], exploded[5], exploded[8]));

			return true;
		}
		case 315: //End of /who list
		{
			status = CS_INCHAN;
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
		callback->serverCallback(SM_RENAME, oldnick+" "+third);
		return true;
	}
	else if (second == "PART") //Part message
	{
		GET_THIRD();
		
		if (third != channel)	//Somewhere we don't care about
			return true;
		
		//get usernick
		string partnick = getNickFromInfo(first);

		//TODO - there should be a part reason, four+ ?
		channelUserLeave(partnick);
		callback->channelChangeCallback(CC_PART, partnick);
		return true;
	}
	else if (second == "JOIN") //Join message
	{
		GET_THIRD_PLUS();
		
		if (third[0] == ':') third = third.substr(1, third.length());
		
		if (third != channel) //channel we don't care aobut
			return true;
		
		string joinnick = getNickFromInfo(first);
		if (joinnick == nick) //its us, get list of people in chan
		{
			sendData("WHO " + channel + "\r\n");
			//TODO -> do we want to send a message you have joined?
			return true;
		}
		
		channelUserLeave(joinnick); //make that user leave if they are already in there
		channelPeople.push_back(ircPerson(first));
		callback->channelChangeCallback(CC_JOIN, joinnick);
		return true;
	}
	else if (second == "QUIT") //Quit message
	{
		GET_THIRD_PLUS();
		
		//get usernick
		string quitnick = getNickFromInfo(first);

		channelUserLeave(quitnick);
		callback->channelChangeCallback(CC_QUIT, quitnick + " " + third);
		return true;
	}
	else if (second == "MODE") //Mode message
	{
		//:danzel!aaa@bbb MODE #hamlan +vv Randy DuEy
		//TODO - doesn't handle multiple voicing/devoicing correctly
		// :scraps.workgroup MODE &bitlbee +t
		//^^ fails on GET_FOURTH <- TODO
		GET_THIRD();
		GET_FOURTH();
		
		string modenick = getNickFromInfo(first);
		
		string applyedTo = currentLineBuf.substr(fourthSpacePos+1);
		bool value = (fourth[0] == '+');
		
		callback->channelModeCallback(modenick, currentLineBuf.substr(secondSpacePos+1));
		
		list<ircPerson>::iterator iter;
		for (iter=channelPeople.begin(); iter != channelPeople.end(); iter++)
		{
			if ((*iter).nick == applyedTo)
			{
				if (fourth[1] == 'v')
				{
					(*iter).voice = value;
					if (value)	callback->channelChangeCallback(CC_VOICE, applyedTo);
					else		callback->channelChangeCallback(CC_DEVOICE, applyedTo);
				}
				else if (fourth[1] == 'o')
				{
					(*iter).op = value;
					if (value)	callback->channelChangeCallback(CC_OP, applyedTo);
					else		callback->channelChangeCallback(CC_DEOP, applyedTo);
				}
				else if (fourth[1] == 'h')
				{
					(*iter).voice = value;
					if (value)	callback->channelChangeCallback(CC_HOP, applyedTo);
					else		callback->channelChangeCallback(CC_DEHOP, applyedTo);
				}
				break;
			}
		}
		return true;
	}
	else if (second == "TOPIC") //Topic Change message
	{
		//:danzel!fff@fff TOPIC #hamlan :safisjdf saeaower sadflksdf
		GET_THIRD();
		GET_FOURTH_PLUS();
		
		string topicnick = getNickFromInfo(first);
		
		callback->channelChangeCallback(CC_TOPIC, topicnick + " " + fourth);
		return true;
	}
	else if (second == "INVITE") //Invite message
	{
		//TODO. Throw away for now
/*		//get usernick
		char* invitenick = getUserNick(first);
		sprintf(outBuffer, "-:- %s invites you to ", invitenick);
		renderMain(outBuffer, COLOR_BLUE);
		printFourthOnwards(COLOR_BLUE);
		free(first);
		free(second);
		free(invitenick);
*/		return true;
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
		
		if (third == nick)	//PM
		{
			callback->privateMsgCallback(sender, fourth);
		}
		else	//Chan
		{
			callback->channelMsgCallback(sender, fourth);
		}
		return true;
	}
	else if (second == "PONG") //pong reply, throw it away
	{
		return true;
	}
	
	return false;
}
void irc::channelUserLeave(const string &theirNick)
{
	list<ircPerson>::iterator iter;
	for (iter=channelPeople.begin(); iter != channelPeople.end(); iter++)
	{
		if ((*iter).nick == theirNick)
		{
			channelPeople.erase(iter);
			break;
		}
	}
}
