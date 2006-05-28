#ifndef INCLUDED_GUI_ACCOUNT_CREATE_TEXT_H
#define INCLUDED_GUI_ACCOUNT_CREATE_TEXT_H

#include "../dlib/guibits/textLineInput.h"

class accountCreateText : public textLineInput
{
public:
	accountCreateText(const unsigned int &nX, const unsigned int &nY);
	
	wstring getText();
	virtual void inputableActivate();
	virtual void inputableDeactivate();
protected:
	int handleEnter();
};


#endif //INCLUDED_GUI_ACCOUNT_CREATE_TEXT_H
