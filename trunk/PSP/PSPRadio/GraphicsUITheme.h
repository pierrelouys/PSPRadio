#ifndef _PSPRADIOGRAPHICSUITHEME_
#define _PSPRADIOGRAPHICSUITHEME_

#include <map>

using namespace std;

struct Point
{
	int x;
	int y;
};

class CGraphicsUIThemeItem
{
public:
	CGraphicsUIThemeItem()
	{
	}
		
	~CGraphicsUIThemeItem()
	{
	}
	
	Point GetSrc(int index)
	{
		return m_pointSrcList[index];
	}
	
	Point GetSize(int index)
	{
		return m_pointSizeList[index];
	}
	
	map<int, Point> m_pointSrcList;
	Point m_pointDst;
	map<int, Point> m_pointSizeList;
};

class CGraphicsUITheme
{
public:
	CGraphicsUITheme();
	~CGraphicsUITheme();
	
	int GetItem(char *szIniTag, CGraphicsUIThemeItem *pItem);
	int GetLetters(char *szIniTag, CGraphicsUIThemeItem *pItem);
	int GetNumbers(char *szIniTag, CGraphicsUIThemeItem *pItem);	
	int GetImagePath(char *szImagePath, int nLength);
	
public:
	int Initialize(char *szFilename);
	void Terminate();
	
	int StringToPoint(char *szPoint, Point *pPoint);
	int StringToPointList(char *szPoint, map<int, Point> *pPointList);
	

private:
	CIniParser *m_pIniTheme;
};

#endif
