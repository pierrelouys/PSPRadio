#include "bitlbee.h"
#include "dlib/render.h"
#include "dlib/util.h"
#include <iostream>
#include <sstream>

bitlbeeAccount::bitlbeeAccount() {}
bitlbeeAccount::bitlbeeAccount(const string &nType, const string &nDetails)
{
	status = BA_OFFLINE;
	type = nType;
	details = nDetails;
}


bitlbeeUser::bitlbeeUser()
{
	text = textBlock(CHAT_AREA_WIDTH, CHAT_AREA_HEIGHT);
	status = BU_OFFLINE;
	stateChangeTime = 0;
}

bitlbeeUser::bitlbeeUser(const wstring &nNick)	//add a new user who is online, in away mode
{
	text = textBlock(CHAT_AREA_WIDTH, CHAT_AREA_HEIGHT);
	nick = nNick;
	status = BU_AWAY;
	stateChangeTime = 0;
	unreadMessages = false;
}


bitlbeeCallback* bitlbeeCallback::theBee = NULL;
bitlbeeCallback* bitlbeeCallback::getBee()
{
	if (theBee == NULL)
		theBee = new bitlbeeCallback();
	return theBee;
}

void bitlbeeCallback::killBee()
{
	delete theBee;
	theBee = NULL;
}

bitlbeeCallback::bitlbeeCallback()
{
	//default variables
	accountChangeCallback = NULL;
	contactChangeCallback = NULL;
	status = BB_OFFLINE;
	myirc = NULL;
	lastPingTime = 0;
	
	//Prepare the server text box, contact list, and text areas for both
	serverBlock = new textBlock(CHAT_AREA_WIDTH, CHAT_AREA_HEIGHT);
	mainTextA = new textArea(CHAT_AREA_WIDTH, CHAT_AREA_HEIGHT, serverBlock);
	mainTextA->moveTo(2,6);
	
	
	addGuiBit(mainTextA);
	
	//prepare newmessage box
	newmsgbit = new newmsgBit("pics/newmsg.png");
	newmsgbit->moveTo(384,216);
	newMessageCount = 0;
	newServerMsg = false;
	addGuiBit(newmsgbit);
}

void bitlbeeCallback::doConnect(const string &server, const int &port, const string &nick, const string &passwd)
{
	yourName = unicodeClean(nick);
	if (status == BB_OFFLINE)
	{
		myirc = new irc(this);
		if (!myirc->doConnect(server , port, nick))
		{
			delete myirc;
			myirc = NULL;
			return;
		}
		
		//Wait till we are in the bitlbee channel
		while (!myirc->inChannel(BITLBEE_CHANNEL))
		{
			myirc->poll();
			SDL_Delay(1);
		}
		
		//BEGINDEBUG
		cout << "In Chan:" << endl;
		list<ircPerson>::const_iterator iter;
		for (iter = myirc->getChannelDetails(BITLBEE_CHANNEL).channelPeople.begin(); 
			iter != myirc->getChannelDetails(BITLBEE_CHANNEL).channelPeople.end();
			iter++)
		{
			cout << (*iter).nick << endl;
		}
		//ENDDEBUG
		
		//Login to bitlbee
		identify(passwd);
		
		//BEGINDEBUG
		cout << "Accounts:" << endl;
		for (unsigned int a =0; a < accounts.size(); a++)
		{
			cout << accounts[a].type << " " << accounts[a].details << endl;
		}
		//ENDDEBUG
		lastPingTime = SDL_GetTicks();
	}
}

void bitlbeeCallback::registrationConnect(const string &server, const int &port, const string &nick, const string &passwd)
{
	if (status == BB_FAIL)
		status = BB_OFFLINE;
	yourName = unicodeClean(nick);
	if (status == BB_OFFLINE)
	{
		myirc = new irc(this);
		if (!myirc->doConnect(server , port, nick))
		{
			delete myirc;
			myirc = NULL;
			return;
		}
		
		//Wait till we are in the bitlbee channel
		while (!myirc->inChannel(BITLBEE_CHANNEL))
		{
			myirc->poll();
			SDL_Delay(1);
		}
		
		//BEGINDEBUG
		cout << "In Chan:" << endl;
		list<ircPerson>::const_iterator iter;
		for (iter = myirc->getChannelDetails(BITLBEE_CHANNEL).channelPeople.begin(); 
			iter != myirc->getChannelDetails(BITLBEE_CHANNEL).channelPeople.end();
			iter++)
		{
			cout << (*iter).nick << endl;
		}
		//ENDDEBUG
		
		//Login+Register to bitlbee
		doRegister(passwd);
		
		lastPingTime = SDL_GetTicks();
	}
}

void bitlbeeCallback::doDisconnect()
{
	if (myirc != NULL)
	{
		lastPingTime = 0;
		myirc->doDisconnect();
		delete myirc;
		myirc = NULL;
		status = BB_OFFLINE;
	}
}

void bitlbeeCallback::identify(const string &password)
{
	serverMsg("identify " + password);
	while (status == BB_OFFLINE)
	{
		myirc->poll();
		usleep(10*1000);
	}
	if (status == BB_FAIL)
	{
		mainTextA->addText(unicodeClean("Failed to connect!\n"), 0xFF0000FF);
		myirc->doDisconnect();
		delete myirc;
		myirc = NULL;
		return;
	}
	//Now get account list
	status = BB_RECEIVING_ACCOUNTS;
	accounts.resize(0);
	serverMsg("account list");
	while (status == BB_RECEIVING_ACCOUNTS)
	{
		myirc->poll();
		usleep(10*1000);
	}
	if (accountChangeCallback != NULL) accountChangeCallback->notifyAccountChange(accounts);
	
	setBitlbeeDefaults();
}

void bitlbeeCallback::doRegister(const string &password)
{
	serverMsg("register " + password);
	while (status == BB_OFFLINE)
	{
		myirc->poll();
		usleep(10*1000);
	}
	if (status == BB_FAIL)
	{
		mainTextA->addText(unicodeClean("Unable to create account, username may be taken\n"), 0xFF0000FF);
		myirc->doDisconnect();
		delete myirc;
		myirc = NULL;
		return;
	}
	
	setBitlbeeDefaults();
}

void bitlbeeCallback::setBitlbeeDefaults()
{
	//Set the default settings on the account
	serverMsg("set auto_connect false");
	serverMsg("set auto_reconnect true");
	serverMsg("set display_namechanges true");
	serverMsg("set auto_reconnect_delay 60");
	serverMsg("set handle_unknown add");
	serverMsg("save");
}

void bitlbeeCallback::signin(int accountNo) //-1 for all
{
	stringstream oss;
	oss << "account on";
	if (accountNo != -1)
	{
		oss << " " << accountNo;
		accounts[accountNo].status = BA_SIGNINGIN;
	}
	else
	{
		for (unsigned int a = 0; a < accounts.size(); a++)
			accounts[a].status = BA_SIGNINGIN;
	}
	if (accountChangeCallback != NULL) accountChangeCallback->notifyAccountChange(accounts);
	serverMsg(oss.str());
}

void bitlbeeCallback::addAccount(bitlbeeAccountType acctype, wstring Wusername, wstring Wpassword)
{
	string unUsername = unUnicode(Wusername);
	string unPassword = unUnicode(Wpassword);
	switch(acctype)
	{
		case BAT_ICQ:
		{
			//account add oscar <handle> <password> login.icq.com
			serverMsg("account add oscar " + unUsername + " " + unPassword + " login.icq.com");
			accounts.push_back(bitlbeeAccount("ICQ", unUsername));
			break;
		}
		case BAT_AOL:
		{
			//account add oscar <handle> <password> login.oscar.aol.com
			serverMsg("account add oscar " + unUsername + " " + unPassword + " login.oscar.aol.com");
			accounts.push_back(bitlbeeAccount("TOC", unUsername));
			serverMsg("account on");
			break;
		}
		case BAT_YAHOO:
		{
			//account add yahoo <handle> <password>
			serverMsg("account add yahoo " + unUsername + " " + unPassword);
			accounts.push_back(bitlbeeAccount("YAHOO", unUsername));
			break;
		}
		case BAT_MSN:
		{
			//account add msn <handle> <password>
			serverMsg("account add msn " + unUsername + " " + unPassword);
			accounts.push_back(bitlbeeAccount("MSN", unUsername));
			break;
		}
		case BAT_GMAIL:
		{
			//account add jabber <handle>@gmail.com <password> talk.google.com:5223:ssl
			serverMsg("account add jabber " + unUsername + "@gmail.com " + unPassword + " talk.google.com:5223:ssl");
			accounts.push_back(bitlbeeAccount("JABBER", unUsername+"@gmail.com"));
			break;
		}
		default:
		{
			cout << "YOU GAVE A BAD TYPE TO bitlbee::addAccount()!!!" << endl;
		}
	}
	#warning this should only turn on the new account, not all accounts
	serverMsg("account on");
	serverMsg("save");
	
	if (accountChangeCallback != NULL) accountChangeCallback->notifyAccountChange(accounts);

}

void bitlbeeCallback::deleteAccount(const string &accountDetail)
{
	serverMsg("account off " + accountDetail);
	serverMsg("account del " + accountDetail);
	
	vector<bitlbeeAccount>::iterator iter;
	for (iter = accounts.begin(); iter != accounts.end(); iter++)
	{
		if ((*iter).details == accountDetail)
		{
			accounts.erase(iter);
			break;
		}
	}
	if (accountChangeCallback != NULL) accountChangeCallback->notifyAccountChange(accounts);
}

void bitlbeeCallback::setAway()
{
	myirc->setAway("away");
	mainTextA->addText(unicodeClean("Set accounts away\n"), COLOR_AWAY_MODE);
}

void bitlbeeCallback::setBack()
{
	myirc->setBack();
	mainTextA->addText(unicodeClean("Set accounts back\n"), COLOR_AWAY_MODE);
}

void bitlbeeCallback::poll()
{
	if (status != BB_OFFLINE) //if online
	{
		//Check all contacts for timeouts
		list< bitlbeeUser >::iterator iter;
		for (iter = chatContacts.begin(); iter != chatContacts.end(); iter++)
		{
			if ((*iter).stateChangeTime != 0 && SDL_GetTicks() - (*iter).stateChangeTime > STATUS_TIMEOUT)
			{
				(*iter).stateChangeTime = 0;
				if ((*iter).status == BU_OFFLINE)
				{
					if ((*iter).unreadMessages)
						updateUnread(-1);
						
					if ((*iter).nick == currentMainWindow) //unlook at them if they are current
					{
						currentMainWindow = wstring();
						mainTextA->setText(serverBlock);
					}
					chatContacts.erase(iter);
					redoContactsBox();
					break;
				}
				//TODO ONLINE
			}
		}
		
		//Check if we need to ping bitlbee server
		//SDL_GetTicks	unsigned int lastPingTime; //we ping the bitlbee server every 20seconds to try not time out
		if (myirc != NULL && lastPingTime != 0 && SDL_GetTicks() - lastPingTime > 20000)
		{
			lastPingTime = SDL_GetTicks();
			myirc->pingServer();
		}
		
	}
	if (myirc != NULL)
		myirc->poll();
}

void bitlbeeCallback::serverMsg(const string &msg)
{
	myirc->sendPM("&bitlbee", msg);
}

void bitlbeeCallback::changeChatTo(const wstring &nNick)
{
	//TODO CHANGE TITLE (AND ADD TITLE)
	if (nNick.length() == 0)
	{
		#warning need to add unread server messages thingy
		mainTextA->setText(serverBlock);
	}
	else
	{
		list< bitlbeeUser >::iterator iter;
		for (iter = chatContacts.begin(); iter != chatContacts.end(); iter++)
		{
			if ((*iter).nick == nNick)
			{
				if ((*iter).unreadMessages)
				{
					updateUnread(-1);
					(*iter).unreadMessages = false;
					redoContactsBox();
				}
				mainTextA->setText(&(*iter).text);
				break;
			}
		}
	}
	currentMainWindow = nNick;
}

wstring bitlbeeCallback::changeChatToInt(const int &offset) //-1 for server, otherwise number in list
{
	//TODO CHANGE TITLE (AND ADD TITLE)
	if (offset == -1)
	{
		#warning need to add unread server messages thingy
		mainTextA->setText(serverBlock);
		currentMainWindow = wstring();
	}
	else
	{
		list< bitlbeeUser >::iterator iter = chatContacts.begin();
		for (unsigned int at = 0; at < chatContacts.size() && iter != chatContacts.end(); at++)
		{
			if (at == (unsigned int)offset)
			{
				if ((*iter).unreadMessages)
				{
					updateUnread(-1);
					(*iter).unreadMessages = false;
					redoContactsBox();
				}
				mainTextA->setText(&(*iter).text);
				return (currentMainWindow = (*iter).nick);
			}
			iter++;
		}
	}
	return wstring();
}

void bitlbeeCallback::messageCurrent(const wstring &message)
{
	if (message.length() == 0) return;
	
	string target;
	if (currentMainWindow.length() == 0)
		target = "&bitlbee";
	else
		target = unUnicode(currentMainWindow);
	myirc->sendPM(target, unUnicode(message));
	mainTextA->addText(yourName + unicodeClean(": "), COLOR_YOUR_NICK);
	mainTextA->addText(message + unicodeClean("\n"), COLOR_MESSAGE);
}

void bitlbeeCallback::getCurrentBuddyDetails()
{
	if (currentMainWindow.length() > 0)
		myirc->whoIs(unUnicode(currentMainWindow));
}

void bitlbeeCallback::blockCurrentBuddy()
{
	if (currentMainWindow.length() > 0)
	{
		serverMsg("block " + unUnicode(currentMainWindow));
		mainTextA->addText(currentMainWindow + unicodeClean(" blocked\n"), COLOR_BLOCK);
	}
}

void bitlbeeCallback::allowCurrentBuddy()
{
	if (currentMainWindow.length() > 0)
	{
		serverMsg("allow " + unUnicode(currentMainWindow));
		mainTextA->addText(currentMainWindow + unicodeClean(" unblocked\n"), COLOR_BLOCK);
	}
}

void bitlbeeCallback::renameCurrentBuddy(const wstring &newNick)
{
	if (currentMainWindow.length() > 0)
		serverMsg("rename " + unUnicode(currentMainWindow) + " " + unUnicode(newNick));
}

void bitlbeeCallback::removeCurrentBuddy()
{
	if (currentMainWindow.length() > 0)
		serverMsg("remove " + unUnicode(currentMainWindow));
}

bool bitlbeeCallback::isTalkingToSomeone()
{
	return (currentMainWindow.length() > 0);
}

///Callbacks

void bitlbeeCallback::serverCallback      (const serverMessageType &type, const string &message)
{
	if (type == SM_NOTICE || type == SM_GENERAL_TEXT || type == SM_IRC_UNK) return;
	
	if (type == SM_QUIT)
	{
		string::size_type xx = message.find(" ");
		string nNick;
		if (xx == string::npos)
			nNick = message;
		else
			nNick = message.substr(0, xx);
		
		cout << "LEAVE -> " << nNick << endl;
		list< bitlbeeUser >::iterator iter;
		wstring newNick = unicodeClean(nNick);
		for (iter = chatContacts.begin(); iter != chatContacts.end(); iter++)
		{
			if ((*iter).nick == newNick)
			{
				(*iter).status = BU_OFFLINE;
				(*iter).stateChangeTime = SDL_GetTicks();
				break;
			}
		}
		
		//HACK PERFORMANCE-- ON JOIN
		redoContactsBox();
		
		return;
	}
	else if (type == SM_RENAME)
	{
		vector<string> exploded = explode(message, ' ');
		wstring wOld = unicodeClean(exploded[0]);
		///Find person in list, copy and remove them.
		bitlbeeUser backup;
		bool found = false;
		unsigned int theirOffset = 0;
		list< bitlbeeUser >::iterator iter;
		for (iter = chatContacts.begin(); iter != chatContacts.end(); iter++)
		{
			if ((*iter).nick == wOld)
			{
				backup = (*iter);
				chatContacts.erase(iter);
				found = true;
				break;
			}
			theirOffset++;
		}
		if (!found)
		{
			cout << "unable to find " << exploded[0] << " for renaming to " << exploded[1] << endl;
			return;
		}
		///rename, reinsert them
		bool added = false;
		backup.nick = unicodeClean(exploded[1]);
		for (iter = chatContacts.begin(); iter != chatContacts.end(); iter++)
		{
			if ((*iter).nick > backup.nick)
			{
				added = true;
				chatContacts.insert(iter,backup);
				break;
			}
		}
		if (!added)
		{
			chatContacts.push_back(backup);
		}
		///if they were selected, change selected
		if (currentMainWindow ==  wOld)
		{
			changeChatTo(unicodeClean(exploded[1]));
		}
		contactChangeCallback->notifyContactRename(wOld, unicodeClean(exploded[1]));
		return;
	}
	else if (type == SM_WHOIS_REALNAME)
	{
		string::size_type spacePos;
		spacePos = message.find(' ');
		mainTextA->addText(unicodeClean(message.substr(0, spacePos) + " is called: " + message.substr(spacePos+1, message.length()) + "\n"), COLOR_WHOIS_MSG);
		return;
	}
	else if (type == SM_WHOIS_SERVER)
	{
		string::size_type spacePos;
		spacePos = message.find(' ');
		mainTextA->addText(unicodeClean(message.substr(0, spacePos) + " is on: " + message.substr(spacePos+1, message.length()) + "\n"), COLOR_WHOIS_MSG);
		return;
	}
	else if (type == SM_WHOIS_AWAY)
	{
		string::size_type spacePos;
		spacePos = message.find(' ');
		mainTextA->addText(unicodeClean(message.substr(0, spacePos) + " is away: " + message.substr(spacePos+1, message.length()) + "\n"), COLOR_WHOIS_MSG);
		return;
	}
	
	cout << "serverCallback(" << type << ", " << message << ")" << endl;
	stringstream oss; //FIXME wtf am i using a sstream here for?
	oss << message << endl;
	if (currentMainWindow.length() == 0)
	{
		string oss_str = oss.str();
		mainTextA->addText(unicodeClean(oss_str), TEXT_NORMAL_COLOR);
		if (type == SM_IRC_DETAILS || type == SM_IRC_ERROR)
			renderGui();
	}
	else
		serverBlock->addText(unicodeClean(oss.str()), TEXT_NORMAL_COLOR);
}
	
void bitlbeeCallback::channelMsgCallback  (const string &channel, const string &who, const string &message)
{
	if (channel != BITLBEE_CHANNEL)
	{
		//Likely got invited to multiperson chat, should handle these in the future
		cout << "MESSAGE FROM BAD CHANNEL: " << channel << endl;
		return;
	}
	
	if (who == "root")
	{
		if (message == "Password accepted" || message == "Password successfully changed")
		{
			status = BB_IDENTIFIED;
			return;
		}
		else if (message == "Incorrect Password" || message == "Incorrect password" || message == "Nick is already registered")
		{
			mainTextA->addText(unicodeClean(message + "\n"), 0xFF0000FF);
			status = BB_FAIL;
			return;
		}
		else if (message == "End of account list")
		{
			status = BB_IDENTIFIED;
			return;
		}
		else if (message.find("No accounts known.") != string::npos) //Don't print this message, replace with cleaner text
		{
			mainTextA->addText(unicodeClean("No accounts detected, press [select] to add one.\n"), 0xFF0000FF);
			return;
		}
		else if (message.find("-") != string::npos && message.substr(message.find("-")+2) == "Logged in")
		{
			//Find the account and set it to online
			string account = message.substr(0, message.find(" ")); //"MSN"
			for (unsigned int a = 0; a < accounts.size(); a++)
			{
				if (accounts[a].type == account)
					accounts[a].status = BA_ONLINE;
			}
			if (accountChangeCallback != NULL) accountChangeCallback->notifyAccountChange(accounts);
		}
		else if (status == BB_RECEIVING_ACCOUNTS) //probably an account, try it
		{
			// 0. MSN, elite.danzel@gmail.com
			// 2. OSCAR, 79254509 on login.icq.com (connected)
			// 3. OSCAR, danzeltest on login.oscar.aol.com
			//"End of account list"
			vector<string> exploded = explode(message, ' ');
/*			int number; char bin; string type; string details;
			stringstream ss(message);
			ss >> number >> bin >> type >> details;
			type = type.substr(0, type.length()-1);
*/			
			if (exploded.size() < 3 || exploded.size() > 6 || exploded[0][exploded[0].size()-1]!='.' || exploded[1][exploded[1].size()-1]!=',')
				return;
			
			if (exploded.size() == 3 || exploded.size() == 4) // no server details
			{
				accounts.push_back(bitlbeeAccount(exploded[1].substr(0, exploded[1].length()-1), exploded[2]));
			}
			else if ((exploded.size() == 5 || exploded.size() == 6) && exploded[1] == "OSCAR,") // server details, OSCAR
			{
				//test for icq else assume aim
				if (exploded[4] == "login.icq.com")
					accounts.push_back(bitlbeeAccount("ICQ", exploded[2]));
				else
					accounts.push_back(bitlbeeAccount("TOC", exploded[2])); //bitlbee calls it TOC so we will too
			}
			//TODO - care about number, they should come in in the right order though....
//			accounts.push_back(bitlbeeAccount(type, details));
			return;
		}
		
		//TODO - messages from people not on buddy list
		/*
<root> Message from unknown handle dapplesnz@hotmail.com on connection MSN(elite.danzel@gmail.com):
<root> NO
		*/
	}
	cout << "channelMsgCallback(" << who << ", " << message << ")" << endl;
	if (currentMainWindow.length() == 0)
		mainTextA->addText(unicodeClean(message+"\n"), TEXT_NORMAL_COLOR);
	else
		serverBlock->addText(unicodeClean(message+"\n"), TEXT_NORMAL_COLOR);
}
	
void bitlbeeCallback::channelModeCallback (const string &channel, const string &whoDone, const string &mode)
{
	//um, care?
	cout << "channelModeCallback(" << whoDone << ", " << mode << ")" << endl;
}

void bitlbeeCallback::channelChangeCallback(const string &channel, const channelChangeType &type, const string &message)
{
	if (channel != BITLBEE_CHANNEL)
	{
		//Likely got invited to multiperson chat, should handle these in the future
		cout << "channelChange FROM BAD CHANNEL: " << channel << endl;
		return;
	}
	
	switch (type)
	{
	case CC_JOIN:
		{
			cout << "JOIN -> " << message << endl;
			//Find the place to put them in the list
			wstring newNick = unicodeClean(message);
			bool added = false;
			list< bitlbeeUser >::iterator iter;
			for (iter = chatContacts.begin(); iter != chatContacts.end(); iter++)
			{
				if ((*iter).nick == newNick)
				{
					added = true;
					(*iter).stateChangeTime = 0;
					(*iter).status = BU_AWAY;
					break;
				}
				if ((*iter).nick > newNick)
				{
					added = true;
					chatContacts.insert(iter,bitlbeeUser(newNick));
					break;
				}
			}
			if (!added)
			{
				chatContacts.push_back(bitlbeeUser(newNick));
			}
			
			//HACK PERFORMANCE-- ON JOIN
			redoContactsBox();
			
			break;
		}
	case CC_VOICE:
		{
			cout << "VOICE -> " << message << endl;
			list< bitlbeeUser >::iterator iter;
			wstring newNick = unicodeClean(message);
			for (iter = chatContacts.begin(); iter != chatContacts.end(); iter++)
			{
				if ((*iter).nick == newNick)
				{
					(*iter).status = BU_ONLINE;
					break;
				}
			}
			
			//HACK PERFORMANCE-- ON JOIN
			redoContactsBox();
			
			break;
		}
	case CC_DEVOICE:
		{
			cout << "DEVOICE -> " << message << endl;
			list< bitlbeeUser >::iterator iter;
			wstring newNick = unicodeClean(message);
			for (iter = chatContacts.begin(); iter != chatContacts.end(); iter++)
			{
				if ((*iter).nick == newNick)
				{
					(*iter).status = BU_AWAY;
					break;
				}
			}
			
			//HACK PERFORMANCE-- ON JOIN
			redoContactsBox();
			
			break;
		}
	case CC_PART:
		{
			//call serverCallback with SM_QUIT so we don't dupe code :)
			serverCallback(SM_QUIT, message);
		}
	default:
		cout << "channelChangeCallback(" << type << ", " << message << ")" << endl;
	}
}

void bitlbeeCallback::privateMsgCallback  (const string &who,             const string &message)
{
	cout << "privateMsgCallbackFIRST(" << who << ", " << message << ")" << endl;
	wstring whoW = unicodeClean(who);
	if (currentMainWindow == whoW) //this is the person we are talking to
	{
		mainTextA->addText(whoW + unicodeClean(": "), COLOR_SEND_NICK);
		mainTextA->addText(unicodeClean(message + "\n"), COLOR_MESSAGE);
		return;
	}
	else	//find the person
	{
		list< bitlbeeUser >::iterator iter;
		for (iter = chatContacts.begin(); iter != chatContacts.end(); iter++)
		{
			if ((*iter).nick == whoW)
			{
				(*iter).text.addText(whoW + unicodeClean(": "), COLOR_SEND_NICK);
				(*iter).text.addText(unicodeClean(message + "\n"), COLOR_MESSAGE);
	
				//light up their name
				if (!(*iter).unreadMessages) updateUnread(+1);
				(*iter).unreadMessages = true;
				redoContactsBox();
				return;
			}
		}
	}
	
	cout << "privateMsgCallback(" << who << ", " << message << ")" << endl;
}

void bitlbeeCallback::redoContactsBox()
{
	cout << "redoContactsBox()" << endl;
	if (contactChangeCallback != NULL)
	{
		contactChangeCallback->notifyContactChange();
	}
}

void bitlbeeCallback::updateUnread(int amount)
{
	newMessageCount += amount;
	if (newMessageCount < 0)
		cout << "UPDATE UNREAD BECAME NEGATIVE :(" << endl;
	
	if (newMessageCount == 0)
		newmsgbit->disable();
	else
		newmsgbit->enable();
}

void bitlbeeCallback::setBitlbeeAccountChangeCallback(bitlbeeAccountChangeCallback* cb)
{
	accountChangeCallback = cb;
}

void bitlbeeCallback::setBitlbeeContactChangeCallback(bitlbeeContactChangeCallback* cb)
{
	contactChangeCallback = cb;
	cb->setContactList(&chatContacts);
}

bitlbeeCallback::~bitlbeeCallback(){
	delete myirc;
	delete newmsgbit;
	delete mainTextA;
	delete serverBlock;
}

/* TODO CONFIGURE USER SETTINGS
set auto_connect false
set display_namechanges

FUTURE: typing_notice true
*/
