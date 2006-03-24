#include <pspsdk.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include "psp_curses.h"

#define printf pspDebugScreenPrintf

#include "cursesbi.h"
#include "cursesbi.c"

#define PSP_SCREEN_WIDTH 480
#define PSP_SCREEN_HEIGHT 272


int env_termsize(int *x, int *y)
{
	*x = PSP_SCREEN_WIDTH / 7;
	*y = PSP_SCREEN_HEIGHT / 8;

	return 0;
}

int pipe_open(int *fdpair)
{
	static int iIndex = 0;
	char name[10];
	iIndex++;

	sprintf(name, "pipe%d", iIndex);

	/**
	* Create a message pipe
	*
	* @param name - Name of the pipe
	* @param part - ID of the memory partition
	* @param attr - Set to 0?
	* @param unk1 - Unknown
	* @param opt  - Message pipe options (set to NULL)
	*
	* @return The UID of the created pipe, < 0 on error
	*/
	SceUID uid = sceKernelCreateMsgPipe(name, PSP_MEMORY_PARTITION_USER, 0, NULL, NULL);

	if (uid >= 0)
	{
		fdpair[0] = uid | 0x1000;
		fdpair[1] = uid | 0x1100;
		return 0;
	}
	else
	{
		pspDebugScreenPrintf("Error creating pipe 0x%x", uid);
		return -1;
	}
}

int pipe_close(int *fdpair)
{
	/**
	* Delete a message pipe
	*
	* @param uid - The UID of the pipe
	*
	* @return 0 on success, < 0 on error
	*/
	int fd1 = fdpair[0] & (~0x1000);
	int fd2 = fdpair[1] & (~0x1100);
	sceKernelDeleteMsgPipe(fd1);
	sceKernelDeleteMsgPipe(fd2);

	return 0;
}

int pipe_read(int fd, void *buf, size_t len)
{
	/**
	* Receive a message from a pipe
	*
	* @param uid - The UID of the pipe
	* @param message - Pointer to the message
	* @param size - Size of the message
	* @param unk1 - Unknown
	* @param unk2 - Unknown
	* @param timeout - Timeout for receive
	*
	* @return 0 on success, < 0 on error
	*/
	int fd1 = fd & (~0x1000);
	return sceKernelReceiveMsgPipe(fd1, buf, len, 0, NULL, NULL);
}

int pipe_write(int fd, void *buf, size_t len)
{
	/**
	* Send a message to a pipe
	*
	* @param uid - The UID of the pipe
	* @param message - Pointer to the message
	* @param size - Size of the message
	* @param unk1 - Unknown
	* @param unk2 - Unknown
	* @param timeout - Timeout for send
	*
	* @return 0 on success, < 0 on error
	*/
	int fd2 = fd & (~0x1100);
	return sceKernelSendMsgPipe(fd2, buf, len, 0, NULL, NULL);
}

#if 0


std_scr *stdscr;
std_scr g_stdsrc;
int initscr()
{

	stdscr = &g_stdsrc;

	pspDebugScreenInit();

	return 1;
}

int move(int a, int b)
{
	return a = b;
}

int getch()
{
	sleep(1);
	return 'a';
}

int addch(char a)
{
	printf("%c", a);
}

int clrtoeol()
{
	return 0;
}

int refresh()
{
	return 0;
}

int addnstr(const char* _str, int len)
{
	printf(_str);
}

int attron(int a)
{
	return a;
}

int attroff(int a)
{
	return a;
}

#endif
