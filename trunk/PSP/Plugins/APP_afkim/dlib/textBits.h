#ifndef INCLUDED_DLIB_TEXTBITS_H
#define INCLUDED_DLIB_TEXTBITS_H

#include "dlib.h"
#include <list>
#include <vector>
using namespace std;
//TODO replace color with a format object specifying color and font

class formattedText
{
public:
	formattedText(const wstring &newText, const unsigned int &newColor);
	wstring text;
	unsigned int color;
	//something font
};

class textLine
{
public:
	textLine();

	//returns the amount of text currently on this line
	unsigned int getTextWidth() const;
	
	void addText(const wstring &text, const unsigned int &color);
	
//PRIVATE :|
	vector<formattedText> texts;
private:
	int textWidth;
};

class textBlock
{
public:
	textBlock(); //BAD DONT USE
	textBlock(const unsigned int &newWidth, const unsigned int &newHeight);
	
	void addText(const wstring &text, const unsigned int &color);

	unsigned int getHeight();
	
	void reset();
//PRIVATE
	list<textLine> lines;
private:
	void addWord(const wstring &text, const unsigned int &color);
	
	unsigned int maxWidth;
	unsigned int maxHeight; //max lines, if 0 then infinite
};

#endif //INCLUDED_DLIB_TEXTBITS_H
