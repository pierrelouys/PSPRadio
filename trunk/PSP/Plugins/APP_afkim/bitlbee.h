#ifndef INCLUDED_BITLBEE_H
#define INCLUDED_BITLBEE_H

#include "irc.h"
#include "dlib/textBits.h"
#include "dlib/guibits/textArea.h"

#include "gui/newmsgBit.h"

#include <vector>
#include <string>
using namespace std;

#define CHAT_AREA_WIDTH  62
#define CHAT_AREA_HEIGHT 21
#define CHAT_SCROLL_DEPTH 11

#define STATUS_TIMEOUT 5000 /*time to timeout a signoff/on*/
enum bitlbeeStatus {
	BB_OFFLINE,
	BB_IDENTIFIED,
	BB_RECEIVING_ACCOUNTS,
	BB_FAIL
};

//Type for creating accounts _only_
enum bitlbeeAccountType {
	BAT_AOL,
	BAT_ICQ,
	BAT_MSN,
	BAT_GMAIL,
	BAT_YAHOO,
	BAT_BITLBEE // <- DO NOT PASS THIS IN TO THE ACCOUNT CREATE FUNCTION
//	BAT_JAB   -- Adding a jabber account will have its own function
};

enum bitlbeeAccountStatus {
	BA_OFFLINE,
	BA_SIGNINGIN,
	BA_ONLINE
	//AWAY
};

class bitlbeeAccount
{
public:
	bitlbeeAccountStatus status;
	string type;
	string details;
	bitlbeeAccount();
	bitlbeeAccount(const string &nType, const string &nDetails);
};

enum bitlbeeUserStatus { BU_ONLINE, BU_AWAY, BU_OFFLINE };
class bitlbeeUser
{
public:
	bitlbeeUser();
	bitlbeeUser(const wstring &nNick);	//add a new user who is online, in away mode
	
	wstring nick;
	textBlock text;
	
	unsigned int stateChangeTime; //time user change status last (0 = who cares) SDL_GetTicks milliseconds
								  //if status==offline and this > 5 then remove from list
								  //THIS IS TODO ON ALL EXCEPT LOGOFF
	bitlbeeUserStatus status;
	bool unreadMessages;
};

///Interfaces you can implement to get updates from bitlbee
//Your Accounts come online/away/offline
class bitlbeeAccountChangeCallback
{
public:
	//Called when:
	// we have fetched account list
	// accounts are set to log in
	// we receive a 'account logged in' message
	
	virtual void notifyAccountChange(const vector < bitlbeeAccount > &accountList) = 0;
	virtual ~bitlbeeAccountChangeCallback() {};
};

class bitlbeeContactChangeCallback
{
public:
	//Called when a contact list something changes.
	virtual void notifyContactChange() = 0;
	//Called when someone is renamed on your contact list (after altering the contact list list)
	virtual void notifyContactRename(wstring oldNick, wstring newNick) = 0;
	virtual void setContactList(list < bitlbeeUser > *contacts) = 0;
	
	virtual ~bitlbeeContactChangeCallback() {};
};


class bitlbeeCallback : public ircCallback
{
public:
	static bitlbeeCallback* getBee();
	static void killBee(); //the opposite of getBee :P
	void doConnect(const string &server, const int &port, const string &nick, const string &passwd);
	void registrationConnect(const string &server, const int &port, const string &nick, const string &passwd);
	void doDisconnect();
	
	void signin(int accountNo); //-1 for all
	void addAccount(bitlbeeAccountType acctype, wstring username, wstring password);
	
	//Away/back
	void setAway();
	void setBack();
	
	void poll();
	
	~bitlbeeCallback();
	
	//Status things
	bitlbeeStatus status;
	vector < bitlbeeAccount > accounts;
	list < bitlbeeUser > chatContacts;
	
	void setBitlbeeAccountChangeCallback(bitlbeeAccountChangeCallback* cb);
	void setBitlbeeContactChangeCallback(bitlbeeContactChangeCallback* cb);

	//change main chat window to show msgs from 'nick' (if "" then server window)
	void changeChatTo(const wstring &nick);
	wstring changeChatToInt(const int &offset); //-1 for server, otherwise number in list, returns name of new selectee
	
	//send a message to whoever the current person is (msgs server if "")
	void messageCurrent(const wstring &message);
	
	//Perform a whois on the person you are currently chatting to, does nothing if server window
	void getCurrentBuddyDetails();
	//Rename the current Buddy (unless server)
	void renameCurrentBuddy(const wstring &newNick);
	
	//Callbacks
	void serverCallback              (const serverMessageType &type,  const string &message);
	virtual void channelMsgCallback  (const string &channel, const string &who,             const string &message);
	virtual void channelModeCallback (const string &channel, const string &whoDone,         const string &mode);
	virtual void channelChangeCallback(const string &channel, const channelChangeType &type, const string &message);
	virtual void privateMsgCallback  (const string &who,              const string &message);
	
	//This is the currently shown text area, could be messages from a person or server
	textArea* mainTextA; //HACK, should be private
protected:
	bitlbeeCallback();

private:
	unsigned int lastPingTime; //we ping the bitlbee server every 20seconds to try not time out
	
	static bitlbeeCallback* theBee;
	
	void serverMsg(const string &msg);	//send a message to the &bitlbee chan
	void redoContactsBox();				//reset the contactsList(A) objects with the current chatContacts list
	
	
	void identify(const string &password);
	void doRegister(const string &password); //same as identify but registers the account

	//Render Stuff
	wstring currentMainWindow; //nickname or ""(server) of main window target
	
	textBlock* serverBlock;	//we want this here?
	
	
	void updateUnread(int amount); //sets the amount of unread messages +/- and enables/disables the newmsgBit
	newmsgBit* newmsgbit;
	int newMessageCount;
	bool newServerMsg;	//<-- should be used for when you get an unknown message.
	
	wstring yourName;
	
	//callbacks
	bitlbeeAccountChangeCallback* accountChangeCallback;
	bitlbeeContactChangeCallback* contactChangeCallback;
};
/* TODO CONFIGURE USER SETTINGS
set auto_connect false
set display_namechanges
TODO messages in pm?
FUTURE: typing_notice true
*/

#define BITLBEE_CHANNEL "&bitlbee"

//colors for rendering contacts list
#define COLOR_USER_ONLINE  0x000000FF
#define COLOR_USER_AWAY    0x888888FF
#define COLOR_USER_OFFLINE 0xa21818FF
#define COLOR_USER_UNREAD  0x5c0077FF

//Colors for messages, 
//SEND_NICK is the color for the other people in the chat
//YOUR_NICK is the color for your nick in a chat
#define COLOR_MESSAGE      0x000000FF
#define COLOR_SEND_NICK    0x085400FF
#define COLOR_YOUR_NICK    0x163c9dFF
#define COLOR_AWAY_MODE    0x0000FFFF
#define COLOR_WHOIS_MSG    0x9000FFFF

#endif //INCLUDED_BITLBEE_H

