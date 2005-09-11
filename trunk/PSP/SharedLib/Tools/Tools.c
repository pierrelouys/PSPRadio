#include <stdio.h>
#include <string.h> 


char *dirname (char *strPath)
{
	if (strPath[0] == '/' && strPath[1] == 0)
		return strPath;
	
	if (strPath && strPath[0] != 0 && strlen(strPath) > 4)
	{
		if (strrchr(strPath, '/'))
			*(strrchr(strPath, '/')) = 0;
		else
			strcpy(strPath, ".");
	}
	else
	{
			strcpy(strPath, ".");
	}
	
	return strPath;
}
//#endif
#if 0
char *psp_dirname (char *strPath)
{
	char *s = NULL;
	
	if (strPath[0] == '/' && strPath[1] == 0)
		return strPath;
	
	s=strPath+strlen(strPath)-1;
	while (s && *s == '/') {
		*s = '\0';
		s=strPath+strlen(strPath)-1;
	}
	s = strrchr(strPath, '/');
	if (s && *s)
		*s = '\0';
		
	if (s)
		return strPath;
	else
		strcpy(strPath, ".");
	
	return strPath;	
}
#endif

char *basename (char *strPath)
{
	char *pRet="";
	
	if (strPath && strPath[0] != 0)
	{
		pRet = strrchr(strPath, '/') + 1;
	}
	
	if (strlen(pRet) <  4)
	{
		pRet="";
	}
	
	return pRet;
}

/** NEED TO IMPLEMENT! */
char *getpass(const char *__prompt)
{
	printf(__prompt);
	return "password";
}

void herror (__const char *__str)
{
}
