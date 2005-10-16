#ifndef _PSPRADIOGRAPHICSUITHEME_
#define _PSPRADIOGRAPHICSUITHEME_

#include <map>

using namespace std;

struct Point
{
	int x;
	int y;
};

class CGraphicsUIPosItem
{
public:
	CGraphicsUIPosItem() 
	{
		m_bEnabled = true;
	};
	
	~CGraphicsUIPosItem() {};
	
	Point m_pointDst;
	Point m_pointSize;
	
	bool m_bEnabled;
};

class CGraphicsUIThemeItem
{
public:
	CGraphicsUIThemeItem() { }		
	~CGraphicsUIThemeItem() { }
	
	Point GetSrc(int index)
	{
		return m_pointSrcMap[index];
	}
	
	int GetIndexFromKey(char key)
	{
		return m_keyToIndexMap[key];
	}
	
	map<int, Point> m_pointSrcMap;
	map<char, int> m_keyToIndexMap;
	Point m_pointDst;
	Point m_pointSize;
};

class CGraphicsUITheme
{
public:
	CGraphicsUITheme();
	~CGraphicsUITheme();
	
	int GetItem(char *szIniTag, CGraphicsUIThemeItem *pItem);
	int GetLettersAndNumbers(char *szIniTagLetters, 
								char *szIniTagNumbers, 
								CGraphicsUIThemeItem *pItem);
	int GetImagePath(char *szImagePath, int nLength);
	
	int GetPosItem(char *szIniTag, CGraphicsUIPosItem *pItem);
	
public:
	int Initialize(char *szFilename);
	void Terminate();
	
	int StringToPoint(char *szPoint, Point *pPoint);
	int StringToPointMap(char *szPoint, map<int, Point> *pPointMap);
	int StringToKeyIndexMap(char *szKey, map<char, int> *pKeyMap);
	
	int StringToPosItem(char *szPos, CGraphicsUIPosItem *pPosItem);

private:
	CIniParser *m_pIniTheme;
};

#endif