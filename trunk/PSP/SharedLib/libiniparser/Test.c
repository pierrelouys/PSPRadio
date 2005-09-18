#include <stdio.h>
#include <string.h> 
#include "iniparser.h"
#include <Tools.h>
#include <assert.h>

#define CFG_FILENAME "test.cfg"

int main (int argc, char **argv)
{
	char strCfgFile[256];
	char strDir[256];
	dictionary *d = NULL;
	
	printf ("argv[0]='%s' \n", argv[0]);
	
	/** test dirname() */
	strcpy(strDir, "/usr/lib");
	assert(strcmp(dirname(strDir), "/usr")==0);
	strcpy(strDir, "/usr/");
	assert(strcmp(dirname(strDir), "/usr")==0);
	strcpy(strDir, "usr");
	assert(strcmp(dirname(strDir), ".")==0);
	strcpy(strDir, "/");
	assert(strcmp(dirname(strDir), "/")==0);
	strcpy(strDir, ".");
	assert(strcmp(dirname(strDir), ".")==0);
	strcpy(strDir, "..");
	assert(strcmp(dirname(strDir), ".")==0);

	
	strcpy(strDir, argv[0]);
	dirname(strDir); /** Retrieve the directory name */
	
	
	sprintf(strCfgFile, "%s/%s", strDir, CFG_FILENAME);
	
	printf ("Opening '%s'\n", strCfgFile);
	
	d = iniparser_new(strCfgFile);
	
	printf ("username='%s' password='%s'\n", 
		iniparser_getstr(d, "USER:USER"),
		iniparser_getstr(d, "USER:PASS"));
	
	iniparser_free(d);
	
	
	return 0;
}
