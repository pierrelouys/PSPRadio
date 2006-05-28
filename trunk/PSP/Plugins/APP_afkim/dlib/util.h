#ifndef INCLUDED_DLIB_UTIL
#define INCLUDED_DLIB_UTIL

#include "dlib.h"
#include <vector>
#include <string>
using namespace std;

void trim(string& s);
void fulltrim(string& s);

vector < wstring > explode(const wstring &s, const wchar_t &e);
vector < string > explode(const string &s, const char &e);

wstring unicodeClean(const string &s);
string unUnicode(const wstring &w);

int nocase_cmp(const string & s1, const string& s2);
#endif

