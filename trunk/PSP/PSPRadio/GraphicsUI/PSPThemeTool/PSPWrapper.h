#ifndef __PSPTHEMEWRAPPER__
#define __PSPTHEMEWRAPPER__

#include "../GraphicsUITheme.h"

class CPSPWrapper
{
public:
	CPSPWrapper(void);
	~CPSPWrapper(void);

	bool Initialize(char *szThemeFileName);
	bool Terminate(void);
	bool Run(void);

private:
	CGraphicsUITheme m_GUI;
};

#endif