#ifndef INCLUDED_GUI_ACCOUNTSSTATUS_H
#define INCLUDED_GUI_ACCOUNTSSTATUS_H

#include "../dlib/guibit.h"
#include "../bitlbee.h"

// MSN(elite.danzel@gmail.com)
// ICQ(79254509)
// JABBER(danzel@jabber.meta.net.nz)
// YAHOO(username)
// TOC(danzeltest) - Logging in: Signon: danzeltest


//2. OSCAR, 79254509 on login.icq.com (connected)
//3. OSCAR, danzeltest on login.oscar.aol.com

class strSurfP {
public:
	strSurfP(const string &nName);
	string name;
	SDL_Surface* surface;
};

//WARNING: moveto on this object sets the top right point! not the top left.
class accountsStatus : public guiBit, bitlbeeAccountChangeCallback
{
public:
	accountsStatus();
	~accountsStatus();
	
	void draw();
	bool needsRedraw();
	
	void notifyAccountChange(const vector < bitlbeeAccount > &nAccountList);


private:
	bool dirty;
	vector < bitlbeeAccount > accountList;
	
	void addPic(const string &nName);
	SDL_Surface* getPic(const string &name);
	vector<strSurfP> surfaces;
};

#endif //INCLUDED_GUI_ACCOUNTSSTATUS_H
