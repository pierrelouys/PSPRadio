#ifndef INCLUDED_GUI_ACCOUNTDELETER_H
#define INCLUDED_GUI_ACCOUNTDELETER_H

#include "../dlib/guibits/selector.h"
#include "../dlib/textBits.h"
#include "../dlib/guibits/textArea.h"
#include "../bitlbee.h"

class accountDeleter : public selector
{
public:
	accountDeleter();
	~accountDeleter();
	
	//From guibit:
	void draw();
	virtual void inputableActivate(); //overriding selector
	
	//From inputable:
	string getInputKey() const;
protected:
	//Text for the list of accounts you can delete
	textBlock listText;
	textArea* text;

	//functions called when things happen in takeInput,
	//the value returned here is the value returned to takeInput
	virtual string fixSelected();
	virtual string pressCross();
	virtual string pressCircle();
	virtual string pressSelect();
private:
	guiBit* bgimage;
	
	bitlbeeCallback* bitlbee;
};

#endif
