#ifndef DLIB_GUI_TEXTAREA
#define DLIB_GUI_TEXTAREA

#include "../guibit.h"
#include "../dlib.h"
#include "../textBits.h"

#include <list>
using namespace std;

//#define ALIGN_TOP 0
//#define ALIGN_BOTTOM 1

//RULES FOR USING textArea
// - NEVER change the textBlock object that textArea is using, it may not redraw and thats bad...
class textArea : public guiBit
{
public:
	textArea(); //DONT USE
	virtual ~textArea();
	textArea(const int &newWidth, const int &newHeight, textBlock* nText); //width/height are amount of chars
	
	//Erase all current text and replace it with this text
	void setText(textBlock* newText);
	
	//Add this text to the end
	void addText(const wstring &newText, const unsigned int &color);
	
	//Get the text that is currently being rendered
	textBlock* getText();
	
	void setAlign(bool align);
	void draw();
	bool needsRedraw() const;
	
	//Reset the area, set redraw on next blit to true (CALL THIS IF YOU EDIT THE TEXTBLOCK YOU BAD BOY)
	void clearArea();
	
	void setLinesToSkip(int newSkip);
private:
	bool alignment;
	int width;
	int height;
	
	int linesToSkip;
	
	textBlock* text;
	
	bool dirty;	//needs redrawing
	
	void addWord(const wstring &word);
};

#endif //DLIB_GUI_TEXTAREA
