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
	
	map<int, Point> m_pointSrcList;
	Point m_pointDst;
	Point m_pointSize;
};

class CGraphicsUITheme
{
public:
	CGraphicsUITheme();
	~CGraphicsUITheme();
	
	int GetItem(char *szIniTag, CGraphicsUIThemeItem *pItem);
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
