#include "textBits.h"


formattedText::formattedText(const wstring &newText, const unsigned int &newColor)
{
	text = newText;
	color = newColor;
}




textLine::textLine()
{
	textWidth = 0;
}

unsigned int textLine::getTextWidth() const
{
	return textWidth;
}

void textLine::addText(const wstring &text, const unsigned int &color)
{
	//if we have no texts or the last color isn't the same make a new text
	if (texts.size() == 0 || texts.back().color != color)
	{
		texts.push_back(formattedText(text, color));
	}
	else
	{
		texts.back().text += text;
	}
	textWidth += text.length();
}


textBlock::textBlock(){}

textBlock::textBlock(const unsigned int &newWidth, const unsigned int &newHeight)
{
	maxWidth = newWidth;
	maxHeight = newHeight;
	
	lines.push_back(textLine());
}

void textBlock::addWord(const wstring &word, const unsigned int &color)
{

	//TODO - what if word is longer than a line!#!@$
	
	int currentLineWidth = lines.back().getTextWidth();
	if (word.length() + currentLineWidth > maxWidth) //word is too long
	{
		//add new line with "word "
		lines.push_back(textLine());
		lines.back().addText(word + (wchar_t)' ', color);
	}
	else	//word fits on the line
	{
		if (word.length() + currentLineWidth + 1 > maxWidth) 
		{
			//if space makes it too long dont add the space, it'll get wrapped next time anyway
			lines.back().addText(word, color);
		}
		else
		{
			lines.back().addText(word + (wchar_t)' ', color);
		}
	}
}

//Add this text to the end
void textBlock::addText(const wstring &text, const unsigned int &color)
{
	wstring currentWord;
	bool inWord = false;
	for (unsigned int a = 0; a < text.length(); a++)
	{
		if (text[a] == '\n')
		{
			if (inWord)
			{
				addWord(currentWord, color);
				currentWord = wstring();
				
				lines.push_back(textLine());
				inWord = false;
			}
			else
			{
				lines.push_back(textLine());
			}
		}
		else if (text[a] == ' ')
		{
			if (inWord)
			{
				addWord(currentWord, color);
				currentWord = wstring();
				inWord = false;
			}
		}
		else //its a char
		{
			inWord = true;
			currentWord += text[a];
		}
	}
	if (inWord)
		addWord(currentWord, color);
		
	//nuke off lines when we are too long
	while (lines.size() > maxHeight * 2) //keep a page of backlog
	{
		lines.pop_front();
	}
}

unsigned int textBlock::getHeight()
{
	return lines.size();
}

void textBlock::reset()
{
	lines.resize(0);
	lines.push_back(textLine());
}
