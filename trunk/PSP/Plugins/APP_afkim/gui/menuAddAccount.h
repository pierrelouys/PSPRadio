#ifndef INCLUDED_GUI_MENUADDACCOUNT_H
#define INCLUDED_GUI_MENUADDACCOUNT_H

#include "../dlib/guibits/selector.h"

class menuAddAccount : public selector
{
public:
	menuAddAccount();
	~menuAddAccount();
	
	//From guibit:
	void draw();
	
	//From inputable:
	string getInputKey() const;
protected:
	//functions called when things happen in takeInput,
	//the value returned here is the value returned to takeInput
	virtual string fixSelected();
	virtual string pressCross();
	virtual string pressCircle();
	virtual string pressSelect();
private:
	guiBit* bgimage;
};

#endif
