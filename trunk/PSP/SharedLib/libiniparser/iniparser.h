/*
 Based upon iniparser by Freek/2005.
 Converted into a C++ library for
 the PSP
 by Raf 9/2005.
 
 -- -

 Based upon libiniparser, by Nicolas Devillard
 Hacked into 1 file (m-iniparser) by Freek/2005
 Original terms following:

 -- -

 Copyright (c) 2000 by Nicolas Devillard (ndevilla AT free DOT fr).

 Written by Nicolas Devillard. Not derived from licensed software.

 Permission is granted to anyone to use this software for any
 purpose on any computer system, and to redistribute it freely,
 subject to the following restrictions:

 1. The author is not responsible for the consequences of use of
 this software, no matter how awful, even if they arise
 from defects in it.

 2. The origin of this software must not be misrepresented, either
 by explicit claim or by omission.

 3. Altered versions must be plainly marked as such, and must not
 be misrepresented as being the original software.

 4. This notice may not be removed or altered.

 */


#ifndef _INIPARSER_H_
#define _INIPARSER_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

class CIniParser
{
public:
	CIniParser(char *strIniFilename)
	{
		m_strIniFilename = strdup(strIniFilename);
		if (m_strIniFilename != NULL)
		{
			m_dict = iniparser_new(m_strIniFilename);
		}
	}
	~CIniParser()
	{
		iniparser_free(m_dict);
		free(m_strIniFilename), m_strIniFilename = NULL;
	}
	
	void Save();

    char *m_strIniFilename;
		
	
	/** Accessors */
	//char *GetKey(char *section, char *key)
	//{
	//	return iniparser_getkey(m_dict, section, key);
	//}
	
	char *GetStr(char *key)
	{
		return iniparser_getstr(m_dict, key);
	}
	
	char *GetString(char *key, char *def)
	{
		return iniparser_getstring(m_dict, key, def);
	}
	
	int GetInteger(char *key, int notfound = -1)
	{
		return iniparser_getint(m_dict, key, notfound);
	}
	
	void SetInteger(char *key, int value)
	{
		char tmp[10];
		sprintf(tmp, "%d", value);
		iniparser_setstr(m_dict, key, tmp);
	}
	
	void SetString(char *key, char *value)
	{
		iniparser_setstr(m_dict, key, value);
	}
	
private:
	struct dictionary 
	{
		int n; /** Number of entries in dictionary */
		int size; /** Storage size */
		char **val; /** List of string values */
		char **key ; /** List of string keys */
		unsigned *hash; /** List of hash values for keys */
	};
	dictionary *m_dict;

	char * strlwc(char * s);
	char * strupc(char * s);
	char * strskp(char * s);
	char * strcrop(char * s);
	char * strstrip(char * s);
	void * mem_double(void * ptr, int size);
	unsigned dictionary_hash(char * key);
	dictionary * dictionary_new(int size);
	void dictionary_del(dictionary * d);
	char * dictionary_get(dictionary * d, char * key, char * def);
	void dictionary_set(dictionary * d, char * key, char * val);
	void dictionary_unset(dictionary * d, char * key);
	void dictionary_dump(dictionary *d, FILE *f);
	void iniparser_add_entry(dictionary * d, char * sec, char * key, char * val);
	int iniparser_getnsec(dictionary * d);
	char * iniparser_getsecname(dictionary * d, int n);
	void iniparser_dump(dictionary * d, FILE * f);
	void iniparser_dump_ini(dictionary * d, FILE * f);
	char * iniparser_getstr(dictionary * d, char * key);
	char * iniparser_getstring(dictionary * d, char * key, char * def);
	int iniparser_getint(dictionary * d, char * key, int notfound);
	double iniparser_getdouble(dictionary * d, char * key, double notfound);
	int iniparser_getboolean(dictionary * d, char * key, int notfound);
	int iniparser_find_entry(dictionary  *   ini, char        *   entry);
	int iniparser_setstr(dictionary * ini, char * entry, char * val);
	void iniparser_unset(dictionary * ini, char * entry);
	dictionary * iniparser_new(char *ininame);
	void iniparser_free(dictionary * d);

};

#endif

