#ifndef __LOG_H
#define __LOG_H

extern int LogOpen(char *path);
extern int LogClose();
extern int LogPrintf(char *fmt, ...);

#endif
