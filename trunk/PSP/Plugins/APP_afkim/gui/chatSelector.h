#ifndef INCLUDED_GUI_CHATSELECTOR_H
#define INCLUDED_GUI_CHATSELECTOR_H

#include "../dlib/guibits/selector.h"
#include "../bitlbee.h"

//The 'buddy list'
class chatSelector : public selector, public bitlbeeContactChangeCallback
{
public:
	chatSelector(const unsigned int &targetX, const unsigned int &targetY, const string &image);
	void draw();
	string getInputKey() const;
	
	//Called when a contact list something changes.
	void notifyContactChange();
	//Called when someone is renamed on your contact list (after altering the contact list list)
	void notifyContactRename(wstring oldNick, wstring newNick);
	void setContactList(list < bitlbeeUser > *newContacts);
protected:
	//functions called when things happen in takeInput,
	//the value returned here is the value returned to takeInput
	virtual string fixSelected();
	virtual string pressCross();
	virtual string pressSelect();

	wstring selectedNick;
	//for the actual contact list!
	textArea* contactsListA;
	textBlock* contactsList;
	int contactListOffset;	//amount contact list is scrolled down by
	
	//updates the contactListOffset variable and scrolls the contactsListA
	void findNewScrollOffset();
	
	list < bitlbeeUser > *contacts;
};

#endif //INCLUDED_GUI_CHATSELECTOR_H
