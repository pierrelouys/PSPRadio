#include <pspsdk.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include "psp_curses.h"
#include <PSPRadio_Exports.h>

#define printf pspDebugScreenPrintf

#include "cursesbi.h"
#include "cursesbi.c"

#define PSP_SCREEN_WIDTH 480
#define PSP_SCREEN_HEIGHT 272


int env_termsize(int *x, int *y)
{
	*x = PSP_SCREEN_WIDTH / 7;
	*y = PSP_SCREEN_HEIGHT / 8;

	return 1;
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
		fdpair[0] = uid;// | 0x1000;
		fdpair[1] = uid;// | 0x1100;

		ModuleLog(LOG_LOWLEVEL, "pipe_open() success. fd=%d, returning fd0=%d fd1=%d\n", uid, fdpair[0], fdpair[1]);
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
	int fd = fdpair[0];// & (~0x1000);
	sceKernelDeleteMsgPipe(fd);

	ModuleLog(LOG_LOWLEVEL, "pipe_close(). Closed fd=%d", fd);

	return 0;
}

int pipe_nonblocking_read(int fd, void *buf, size_t len)
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
	int fd1 = fd;// & (~0x1000);
	int ret = 0;

    ret = sceKernelTryReceiveMsgPipe(fd1, buf, len, 0, 0);//NULL, NULL);
	
	if (ret == 0)
	{
		ModuleLog(LOG_LOWLEVEL, "pipe_nonblocking_read() fd=%d returned success", fd1);
		return len;
	}
	else if (ret == SCE_KERNEL_ERROR_MPP_EMPTY)
	{
		return 0;
	}
	else
	{
		ModuleLog(LOG_ERROR, "pipe_nonblocking_read(): ReceiveMsgPipe(fd = %d) returned 0x%x (%d)", fd1, ret, ret);
		return -1;
	}
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
	int fd1 = fd;// & (~0x1000);
	int ret = 0;

    ret = sceKernelReceiveMsgPipe(fd1, buf, len, 0, NULL, NULL);
	
	if (ret == 0)
	{
		ModuleLog(LOG_LOWLEVEL, "pipe_read() fd=%d returned success", fd1);
		return len;
	}
	else
	{
		ModuleLog(LOG_ERROR, "pipe_read(): ReceiveMsgPipe(fd = %d) returned 0x%x (%d)", fd1, ret, ret);
		return -1;
	}
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
	int fd2 = fd;// & (~0x1100);
	int ret = 0;
	ret = sceKernelSendMsgPipe(fd2, buf, len, 0, NULL, NULL);

	if (ret == 0)
	{
		ModuleLog(LOG_LOWLEVEL, "pipe_write() fd=%d returned success", fd2);
		return len;
	}
	else
	{
		ModuleLog(LOG_ERROR, "pipe_write(): SendMsgPipe(fd = %d) returned 0x%x (%d)", fd2, ret, ret);
		return -1;
	}
}
