#ifndef INCLUDED_GUI_ACCOUNT_CREATE_TEXT_H
#define INCLUDED_GUI_ACCOUNT_CREATE_TEXT_H

#include "../dlib/guibits/textLineInput.h"

class accountCreateText : public textLineInput
{
public:
	accountCreateText(const unsigned int &nX, const unsigned int &nY, const string &inputKey, const string &nextInput);
	
	wstring getText();
	
	//From inputable
	string getInputKey() const;
	virtual void inputableActivate();
	virtual void inputableDeactivate();
protected:
	string handleEnter();
private:
	string myInputKey;
	string nextInputKey;
};


#endif //INCLUDED_GUI_ACCOUNT_CREATE_TEXT_H
