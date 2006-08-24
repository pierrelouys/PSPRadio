
#define PSP_NET_MODULE_COMMON 1
#define PSP_NET_MODULE_ADHOC 2
#define PSP_NET_MODULE_INET 3
#define PSP_NET MODULE_PARSEURI 4
#define PSP_NET_MODULE_PARSEHTTP 5
#define PSP_NET_MODULE_HTTP 6
#define PSP_NET_MODULE_SSL 7


# ifdef __cplusplus
extern "C" {
# endif

extern int sceUtilityLoadNetModule(int);
extern int sceUtilityUnloadNetModule(int);

extern int pspUserLoadInetModules();
extern int pspUserInetInit();
	
# ifdef __cplusplus
}
# endif

#define pspSdkLoadInetModules pspUserLoadInetModules
#define pspSdkInetInit pspUserInetInit
