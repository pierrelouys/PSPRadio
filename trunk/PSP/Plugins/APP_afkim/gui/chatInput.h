#ifndef INCLUDED_LIB_CHATINPUT_H
#define INCLUDED_LIB_CHATINPUT_H

#include "../dlib/guibits/textLineInput.h"

class chatInput : public textLineInput
{
public:
	chatInput(const unsigned int &nLength, const unsigned int &nX, const unsigned int &nY, const unsigned int &nColor);
	void inputableActivate();
	void inputableDeactivate();
protected:
	int handleEnter();
};

#endif //INCLUDED_LIB_CHATINPUT_H
