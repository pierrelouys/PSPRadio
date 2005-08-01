#ifndef _TOOLS_
#define _TOOLS_

char *dirname (char *strPath, char *strBufOut);
char *basename (char *strPath);

#define sigsetjmp(a,b) printf("not calling sigsetjmp")
#define siglongjmp(a,b)
#define jabort 1

#define PATH_MAX MAXPATHLEN
#define SO_DEBUG 1

#endif
