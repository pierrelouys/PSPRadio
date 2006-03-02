#define MAX_USER_LENGTH 50
#define MAX_PASS_LENGTH 50
#define MAX_PATH_LENGTH 1024
#define TRANSFER_BUFFER_SIZE 4096
#define MAX_COMMAND_LENGTH 1024

typedef struct MftpConnection {
	int comSocket;
	int dataSocket;
	int pasvSocket;

	char comBuffer[1024];
	char dataBuffer[1024];

	char serverIp[32];

	char root[MAX_PATH_LENGTH];
	char curDir[MAX_PATH_LENGTH];

	char transferType;
	char user[MAX_USER_LENGTH];
	char pass[MAX_PASS_LENGTH];

	int usePASV;
	int userLoggedIn;

	int port_port;
	union
	{
		char port_addr_c[4];
		struct in_addr port_addr;
		//struct sockaddr port_addr;
	};
} MftpConnection;

int mftpClientHandler(SceSize args, void *argp);

