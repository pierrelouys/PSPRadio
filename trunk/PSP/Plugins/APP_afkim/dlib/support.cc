#include "support.h"

#include <ft2build.h>
#include FT_FREETYPE_H
 
FT_Library    library;
FT_Face       face;
FT_GlyphSlot  slot;

void* fontFile;

void support_init()
{
	/*error = */FT_Init_FreeType( &library );              /* initialize library */
	/* error handling omitted */
	
	//load into memory and use there, SOOO SLOW off disk :|
	FILE * inFile;
	inFile = fopen("./font.ttf", "rb");
	fseek(inFile, 0, SEEK_END);
	int size = ftell(inFile);
	fontFile = malloc(size+1);
	rewind(inFile);
	fread(fontFile, 1, size, inFile);
	fclose(inFile);
	
	FT_New_Memory_Face(library, (FT_Byte*)fontFile, size, 0, &face);
	/* error handling omitted */
	/*error = */FT_Set_Pixel_Sizes(face, 10, 10);
	/* error handling omitted */
	
	//Select the unicode map, doesn't seem to do anything *shrug*
	FT_CharMap found = 0; 
	FT_CharMap charmap; 
	int x = 0; 
	for ( x = 0; x < face->num_charmaps; x++ ) 
	{
		charmap = face->charmaps[x]; 
//		printf("%i -- %i,%i\n", x, charmap->platform_id, charmap->encoding_id);
		if ( charmap->platform_id == 3 &&  charmap->encoding_id == 1) //unicode
		{ found = charmap; break; } 
	}
	if (found) FT_Set_Charmap( face, found );

	slot = face->glyph;
}

struct cachedChar
{
	unsigned int char_code;
	
	//offsets in the drawing area calculated full_font_sizes - this char size
	char ox;
	char oy;
	
	char width;
	char height;
	
	unsigned char* buf;
	
	struct cachedChar* nextChar;
};

struct cachedChar* newChar(unsigned int char_code)
{
	FT_Vector     pen;                    /* untransformed origin  */
	pen.x = 0;
	pen.y = 0;
	
	/* set transformation */
	FT_Set_Transform( face, NULL, &pen );
//	/*error = */FT_Load_Char( face, FT_Get_Char_Index(face,char_code), FT_LOAD_RENDER );
	/*error = */FT_Load_Char( face, char_code, FT_LOAD_RENDER );
	
	//Get a new character and copy the char in
	struct cachedChar* newChar = (struct cachedChar*)malloc(sizeof(struct cachedChar));
	newChar->nextChar = 0;
	newChar->char_code = char_code;
	
	newChar->ox = slot->bitmap_left;
	newChar->oy = 10 - slot->bitmap_top;
	
	newChar->width  = slot->bitmap.width;
	newChar->height = slot->bitmap.rows;
	
	newChar->buf = (unsigned char*)malloc(newChar->width * newChar->height);
	memcpy(newChar->buf, slot->bitmap.buffer, newChar->width * newChar->height);
	
	return newChar;
}

struct cachedChar* cacheFront = 0;
int cacheSize = 0;
#define MAX_CACHE_SIZE 128

struct cachedChar* getChar(FT_ULong char_code)
{
	//TODO find in cache
	if (cacheFront == 0)
	{
		cacheSize++;
		return (cacheFront = newChar(char_code));
	}
	else
	{
		if (cacheFront->char_code == char_code)
			return cacheFront;
		
		//look through whole cache
		struct cachedChar *a = cacheFront;
		while (a->nextChar != 0)
		{
			if (a->nextChar->char_code == char_code)
			{
				//record the found one and rearrange the list to put it at the front
				struct cachedChar* correctOne = a->nextChar;
				a->nextChar = correctOne->nextChar;
				correctOne->nextChar = cacheFront;
				cacheFront = correctOne;
				return cacheFront;
			}
			a = a->nextChar;
		}
		//Char not found, add it at front
		struct cachedChar* new_char = newChar(char_code);
		new_char->nextChar = cacheFront;
		cacheFront = new_char;
		
		cacheSize++;
		
		//cache is too big, remove the last item
		if (cacheSize > MAX_CACHE_SIZE)
		{
			struct cachedChar* aChar = cacheFront;
			while(aChar->nextChar->nextChar != 0)
				aChar = aChar->nextChar;
			//now we've found the one before the last one, so nuke it off
			free(aChar->nextChar->buf);
			free(aChar->nextChar);
			aChar->nextChar = 0;
			cacheSize--;
		}
		
		return cacheFront;
	}
}

//unsigned int color = 0xFFFFFFFF;

void putChar(unsigned int* pixels, const int &pixel_width, const int &posx,const int &posy, unsigned int character, unsigned int color)
{
	if (posy < 0) //dont draw off the -'ve y
		return;
//	printf("drawing %c at %i*%i\n", character, posx, posy);
	int ax, ay; //point on the font we are drawing

	struct cachedChar* theChar = getChar(character);
	
	for (ay = 0; ay < theChar->height; ay++)
	{
		for (ax = 0; ax < theChar->width; ax++)
		{
			unsigned int pixAt = theChar->buf[ay * theChar->width + ax];
			
			unsigned int bg = pixels[(posx + ax + theChar->ox) + ((posy + ay + theChar->oy) * pixel_width)];
			unsigned int finalColor = 0;
			color = (color & 0xFFFFFF00) | pixAt;
			int top_trans = 0xFF & color;
			int  bg_trans = 0xFF & bg;
			if (bg_trans == 0)
			{
//				printf("quick ");
				finalColor = color;
			}
			else
			{
				finalColor = 
/*r*/	(((top_trans * ((0xFF000000 & color) >> 24)) + ((0xFF-top_trans) * ((0xFF000000 & bg) >> 24))>>8) << 24) |
/*g*/	(((top_trans * ((0x00FF0000 & color) >> 16)) + ((0xFF-top_trans) * ((0x00FF0000 & bg) >> 16))>>8) << 16) |
/*b*/	(((top_trans * ((0x0000FF00 & color) >>  8)) + ((0xFF-top_trans) * ((0x0000FF00 & bg) >>  8))>>8) <<  8) |
/*t*/	((top_trans));
			}
			
			pixels[(posx + ax + theChar->ox) + ((posy + ay + theChar->oy) * pixel_width)] = finalColor;
		}
	}
}

#define LOCKPIXELS(); 	if (SDL_MUSTLOCK(pixels)) SDL_LockSurface(pixels);
#define UNLOCKPIXELS();	if (SDL_MUSTLOCK(pixels)) SDL_UnlockSurface(pixels);

void printTextToSurface(textBlock &text, SDL_Surface* pixels, bool align, const int &linesToSkip)
{
	LOCKPIXELS();
	
	int linesProcessed = 0;
	
	if (align == ALIGN_BOTTOM)
		text.lines.reverse();
	
	list<textLine>::const_iterator iter;
	iter=text.lines.begin();
	//HACK-ISH skip the first line if its blank
	if ((*iter).getTextWidth() == 0)
		iter++;
	
	//skip the amount of lines specified
	//FIXME: THIS COULD CRASH
	for (int a = 0; a < linesToSkip; a++)
		iter++;
	
	for (; iter != text.lines.end(); iter++)//each line
	{
		int charsAcross = 0;
		for (unsigned int a = 0; a < (*iter).texts.size(); a++) //for each block of text
		{
			for (unsigned int b = 0; b < (*iter).texts[a].text.length(); b++)//each char in the block
			{
				if (align == ALIGN_BOTTOM)
					putChar((unsigned int*)pixels->pixels, pixels->w, charsAcross*FONT_WIDTH, (pixels->h - ((1+linesProcessed)*FONT_HEIGHT)-FONT_EXTRA_HEIGHT), (*iter).texts[a].text[b], (*iter).texts[a].color);
				else // ALIGN_TOP
					putChar((unsigned int*)pixels->pixels, pixels->w, charsAcross*FONT_WIDTH, ((linesProcessed*FONT_HEIGHT)), (*iter).texts[a].text[b], (*iter).texts[a].color);
					
				charsAcross++;
			}
		}
		
		linesProcessed++;
		if (linesProcessed >= (pixels->h/FONT_HEIGHT)) break;
	}
	
	if (align == ALIGN_BOTTOM)
		text.lines.reverse();
	
	UNLOCKPIXELS();
}
