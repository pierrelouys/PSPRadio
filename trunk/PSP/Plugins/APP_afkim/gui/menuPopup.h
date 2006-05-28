#ifndef INCLUDED_GUI_MENUPOPUP_H
#define INCLUDED_GUI_MENUPOPUP_H

#include "../dlib/guibits/selector.h"

class menuPopup : public selector
{
public:
	menuPopup();
	~menuPopup();
	void draw();
	int changeReturnVal;
protected:
	//functions called when things happen in takeInput,
	//the value returned here is the value returned to takeInput
	virtual int fixSelected();
	virtual int pressCross();
	virtual int pressSelect();
private:
	guiBit* bgimage;
};

#endif
