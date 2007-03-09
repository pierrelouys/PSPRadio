#ifndef INCLUDED_GUI_MENUPOPUP_H
#define INCLUDED_GUI_MENUPOPUP_H

#include "../dlib/guibits/selector.h"

class menuPopup : public selector
{
public:
	menuPopup();
	~menuPopup();
	string changeReturnVal;
	
	//From guibit:
	void draw();
	
	//From inputable:
	string getInputKey() const;
protected:
	//functions called when things happen in takeInput,
	//the value returned here is the value returned to takeInput
	virtual string fixSelected();
	virtual string pressCross();
	virtual string pressSelect();
private:
	guiBit* bgimage;
};

#endif
