#ifndef INCLUDED_PROTOCOL_IRC_H
#define INCLUDED_PROTOCOL_IRC_H

#include <list>
#include <string>
#include <netinet/in.h>
using namespace std;

class irc;

//Status of irc connection
enum connectionStatus {
	CS_OFFLINE,		//Not connected at all
	CS_CONNECTING,	//Attempting to connect
	CS_IDENTIFYING,	//connected to server, registering nick and waiting for okay message
	CS_CONNECTED,	//Really connected to server, do what you want
	CS_INCHAN		//In the channel and received user list
};

//Types of message that a server message could contain
enum serverMessageType {
	SM_IRC_UNK,        //unknown line from the irc class
	SM_IRC_DETAILS,    //message from the irc class on status etc
	SM_IRC_ERROR,      //error message from the irc class
	SM_GENERAL_TEXT,   //general text from the server, MOTD etc
	SM_NOTICE,         //NOTICE message from the server
	SM_RENAME,         //Message format "oldnick newnick", this is a nick change.
	SM_NONFATAL_ERROR, //an error message of some kind, not fatal (your last command probably failed)
	SM_NICK_TAKEN,     //The nick you are trying to use is already taken. (msg = attempted nick)
	SM_UNKNOWN
};

enum channelChangeType {
	CC_TOPIC,	//Message format "nick topic", topic may contain spaces
	CC_JOIN,
	CC_PART,
	CC_QUIT,	//Message format "nick reason", reason may contain spaces
	CC_VOICE,
	CC_DEVOICE,
	CC_OP,
	CC_DEOP,
	CC_HOP,
	CC_DEHOP
};


//Class to receive all callbacks from an irc object
class ircCallback
{
public:
	//Messages from the server you may care about
	virtual void serverCallback       (const serverMessageType &type, const string &message) = 0;
	
	//Message in a channel from someone
	virtual void channelMsgCallback   (const string &who,             const string &message) = 0;
	
	//Channel mode changes
	//"root"  "+v jimmy"
	//These are sent before a channelChangeCallback for each of the actual changes
	virtual void channelModeCallback  (const string &whoDone,         const string &mode) = 0;
	
	//People join/leave chan
	virtual void channelChangeCallback(const channelChangeType &type, const string &message) = 0;
	
	//Private message from someone
	virtual void privateMsgCallback   (const string &who,             const string &message) = 0;
	
	virtual ~ircCallback() {};
	irc* myirc; //should be static or put it else where as static
};

class ircPerson
{
public:
	ircPerson(); //DONT USE
	ircPerson(string joinInfo); // nick!user@host
	ircPerson(const string &nNick, const string &nRealname, const string &nUser, const string &nHost, const string &nMode);
	
//don't fuck with these
	bool voice;
	bool op;
	bool hop;
	
	string nick;
	string realname;
	bool knowrealname;
		
	string user;	// user@
	string host;	//     @mail.com
};

//Currently a single channel irc class
class irc
{
public:
	irc(const string &nServer, const int &nPort, const string &nChannel, const string &nNick, ircCallback* CB);
	
	//Connect to an irc server, blocking
	//May generate multiple callbacks while running
	//if returns true then it is connected to a server and is running
	bool doConnect();
	
	//Disconnects from the server, nonblocking(?)
	void doDisconnect();
	
	//Poll the connection for waiting messages, blocking-ish
	//may generate callbacks if something has happened
	void poll();
	
	//send message to target (works for channels too), 
	void sendPM(const string &target, const string &message);
	//send a raw command to the server (\r\n is automatically appended)
	void sendRaw(const string &message);
	void pingServer(); //non blocking, throws away output... not really a ping at all, lol
	
	//Away Modes
	void setAway(const string &message);
	void setBack();
	
	//List of all people in the channel
	list<ircPerson> channelPeople;
private:
	void sendData(const string &data);
	
	void processLineBuf();
	bool parseFirst();
	bool parseSecond();
	bool parseSecondString();
	
	void channelUserLeave(const string &theirNick);
	
	//User Settings
	string server;
	int port;
	string channel;
	string nick;

	//Callbacks
	ircCallback* callback;
	
	//Connection
	int sock;
	
	connectionStatus status;
	
	string currentLineBuf;
};

#endif
