#include <stdio.h>
#include <string.h> 

char *dirname (char *strPath, char *strBufOut)
{
	if (strPath && strPath[0] != 0 && strlen(strPath) > 4)
	{
		strcpy(strBufOut, strPath);
		if (strrchr(strBufOut, '/'))
			*(strrchr(strBufOut, '/') + 1) = 0;
	}
	else
	{
		strcpy(strBufOut, "ms0:/");
	}
	
	return strBufOut;
}

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