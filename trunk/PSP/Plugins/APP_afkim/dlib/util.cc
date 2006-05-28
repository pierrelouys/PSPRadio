#include "util.h"

void trim(string& s)
{
	unsigned int a;
	for(a=0; a < s.length();a++)
	{
		if (s[a] != ' ' && s[a] != '\n' && s[a] != '\t' && s[a] != '\r') break;
	}
	if (a == s.length())
	{
		s = "";
		return;
	}
	else if (a > 0)
	{
		s.erase(0,a-1);
		s[0] = ' ';
	}
		
	for(a=s.length()-1; a >= 0;a--)
	{
		if (s[a] != ' ' && s[a] != '\n' && s[a] != '\t' && s[a] != '\r') break;
	}
	if (a < s.length()-1)
	{
		s.erase(a+1);
		s.append(" ");
	}
}
#include <iostream>
void fulltrim(string& s)
{
//	cout << "FT:" << s << ".\n";
	unsigned int a;
	for(a=0; a < s.length();a++)
	{
		if (s[a] != ' ' && s[a] != '\t' && s[a] != '\n' && s[a] != '\r')
			break;
/*		if (((s[a] >= 'a' && s[a] <= 'z')
			||
			(s[a] >= 'A' && s[a] <= 'Z')) && s[a] != 'm'
		) break;
*/	}
	if (a == s.length())
		s = "";
	else if (a > 0)
	{
		s.erase(0,a);
	}
	
	if (s.length() == 0) return;
	
	for(a=s.length()-1; a > 0;a--)
	{
		if (s[a] != ' ' && s[a] != '\n' && s[a] != '\t') break;
	}
	if (a < s.length()-1)
	{
		s.erase(a+1);
	}
}

vector < wstring > explode(const wstring &s, const wchar_t &e)
{
	vector < wstring > exploded;
	exploded.push_back( wstring() );
	bool parsing = false;
	
	for (unsigned int a = 0; a < s.size(); a++)
	{
		if (s[a]==e)
		{
			if (!parsing)
				continue;	//if we aren't currently in a string and we find the break char, skip it
			else
			{
				exploded.push_back( wstring() );
				parsing = false;
			}
		}
		else
		{
			exploded.back().push_back(s[a]);
			parsing = true;
		}
	}
	if (exploded.back().length() == 0)
		exploded.pop_back();
	
	return exploded;
}

vector < string > explode(const string &s, const char &e)
{
	vector < string > exploded;
	exploded.push_back( string() );
	bool parsing = false;
	
	for (unsigned int a = 0; a < s.size(); a++)
	{
		if (s[a]==e)
		{
			if (!parsing)
				continue;	//if we aren't currently in a string and we find the break char, skip it
			else
			{
				exploded.push_back( string() );
				parsing = false;
			}
		}
		else
		{
			exploded.back().push_back(s[a]);
			parsing = true;
		}
	}
	if (exploded.back().length() == 0)
		exploded.pop_back();
	
	return exploded;
}




/*
	TODO future
	
	find &
	find next ;
	str-rep on the inside, much nicer
	
	http://msdn.microsoft.com/workshop/author/dhtml/reference/charsets/charset3.asp
	http://www.w3.org/TR/2002/REC-xhtml1-20020801/dtds.html#h-A2
*/
#include <iostream>
void swap(string &s, const string &from, const string &to)
{
	string::size_type found = 1;
	while ((found = s.find(from, 0)) != string::npos)
	{
//		cout << "Replacing" << endl;
		s.replace(found, from.length(), to);
	}
}

static const char trailingBytesForUTF8[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};
static const unsigned int offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL, 
		     0x03C82080UL, 0xFA082080UL, 0x82082080UL };

wstring unicodeClean(const string &s)
{
	const char* sourceStart = s.c_str();
	unsigned const char* sourceEnd =(unsigned char*) sourceStart + s.length();

	unsigned char * source = (unsigned char*)sourceStart;
	wstring retval;
	while (source < sourceEnd)
	{
		wchar_t ch = 0;
		unsigned short extraBytesToRead = trailingBytesForUTF8[(unsigned char)*source];
/*		if (source + extraBytesToRead >= sourceEnd) {
			result = sourceExhausted; break;
		}
*/		/* Do this check whether lenient or strict */
/*		if (! isLegalUTF8(source, extraBytesToRead+1)) {
			result = sourceIllegal;
			break;
		}*/
		/*
		* The cases all fall through. See "Note A" below.
		*/
		switch (extraBytesToRead) {
			case 5: ch += *source++; ch <<= 6;
			case 4: ch += *source++; ch <<= 6;
			case 3: ch += *source++; ch <<= 6;
			case 2: ch += *source++; ch <<= 6;
			case 1: ch += *source++; ch <<= 6;
			case 0: ch += *source++;
		}
		ch -= offsetsFromUTF8[extraBytesToRead];
	
// 		if (target >= targetEnd) {
// 			source -= (extraBytesToRead+1); /* Back up the source pointer! */
// 			result = targetExhausted; break;
//		}

//		if (ch <= UNI_MAX_LEGAL_UTF32) {
			/*
			* UTF-16 surrogate values are illegal in UTF-32, and anything
			* over Plane 17 (> 0x10FFFF) is illegal.
			*/
//			if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) {
//				if (flags == strictConversion) {
//					source -= (extraBytesToRead+1); /* return to the illegal value itself */
//					result = sourceIllegal;
//					break;
//				} else {
//					*target++ = UNI_REPLACEMENT_CHAR;
//				}
//			} else {
//				*target++ = ch;
//			}
//		} else { /* i.e., ch > UNI_MAX_LEGAL_UTF32 */
//			result = sourceIllegal;
//			*target++ = UNI_REPLACEMENT_CHAR;
//		}
		retval += ch;
	}
//	*sourceStart = source;
//	*targetStart = target;

#define REPLACE(x,y) for (unsigned int a=0; a < retval.size();a++) if (retval[a]==x) retval[a]=y;
	//Now we will clean up all the characters that need to be converted from html numbers to readable chars
	//TODO: find out if there is a better place/way to do this
	REPLACE(146, 0x2019); // '
	REPLACE(147, 0x201C); // opening "
	REPLACE(148, 0x201D); // closing "
	REPLACE(151, 0x2014); // -
	
	return retval;
}

string unUnicode(const wstring &w)
{
	//TODO this is a HACK
	string retval;
	for (unsigned int a = 0; a < w.length(); a++)
	{
		retval += (char)w[a];
	}
	return retval;
}

int nocase_cmp(const string & s1, const string& s2) 
{
	string::const_iterator it1=s1.begin();
	string::const_iterator it2=s2.begin();
	
	//stop when either string's end has been reached
	while ( (it1!=s1.end()) && (it2!=s2.end()) ) 
	{
		if(::toupper(*it1) != ::toupper(*it2)) //letters differ?
			// return -1 to indicate smaller than, 1 otherwise
			return (::toupper(*it1)  < ::toupper(*it2)) ? -1 : 1; 
		//proceed to the next character in each string
		++it1;
		++it2;
	}
	size_t size1=s1.size(), size2=s2.size();// cache lengths
	//return -1,0 or 1 according to strings' lengths
	if (size1==size2) 
		return 0;
	return (size1<size2) ? -1 : 1;
}
