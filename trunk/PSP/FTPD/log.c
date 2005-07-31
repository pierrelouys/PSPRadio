#include <pspkernel.h>
#include <stdio.h>      
#include <stdarg.h>  
#include "log.h"

int g_fdLog = 0;

int LogOpen(char *path) {
  g_fdLog = sceIoOpen(path, PSP_O_CREAT|PSP_O_RDWR|PSP_O_TRUNC, 0777);
  return g_fdLog;
}

int LogClose() {
  if(g_fdLog)
    sceIoClose(g_fdLog);
  return 0;
}

int LogPrintf(char *fmt, ...) {
  va_list opt;
  
  char buff[2048];
  int bufsz;
  
  va_start(opt, fmt);
  bufsz = vsnprintf( buff, (size_t) sizeof(buff), fmt, opt);

  sceIoWrite(g_fdLog, buff, bufsz);

  return 0;
}
