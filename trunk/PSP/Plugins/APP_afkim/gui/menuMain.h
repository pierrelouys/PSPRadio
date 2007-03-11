#ifndef INCLUDED_GUI_MENUMAIN_H
#define INCLUDED_GUI_MENUMAIN_H

#include "../dlib/guibits/selector.h"

class menuMain : public selector
{
public:
	menuMain();
	~menuMain();
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
