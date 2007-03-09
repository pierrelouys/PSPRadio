#ifndef INCLUDED_LIB_CHATINPUT_H
#define INCLUDED_LIB_CHATINPUT_H

#include "../dlib/guibits/textLineInput.h"

class chatInput : public textLineInput
{
public:
	chatInput(const unsigned int &nLength, const unsigned int &nX, const unsigned int &nY, const unsigned int &nColor);
	string getInputKey() const;
	void inputableActivate();
	void inputableDeactivate();
protected:
	string handleEnter();
};

#endif //INCLUDED_LIB_CHATINPUT_H
