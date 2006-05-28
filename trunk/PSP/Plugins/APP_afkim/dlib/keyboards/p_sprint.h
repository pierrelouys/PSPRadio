
struct p_sp_Key
{
	int keyid;
	int keychar;
	int keycode;
	int modifiers;
	int keygroup;
};

int p_spReadKeyEx(struct p_sp_Key * newKey, unsigned int Buttons);
/* 	non-blocking routine that sets a key structure containing
	a keyid, PC compatible keyCode, PC compatile keyChar, 
	PC compatible modifier state (shift,control, alt) and
	the key group that was selected by the user.

	The 'Ex' version is specifically designed to cooperate with a
	mouse driver. You can do your own reads from the controller
	through the SceCtrlData interface, and pass on the buttons
	you don't use yourself to p-sprint to see if they result in 
	a keypress. 
	
	P-sprint is designed to work together with a mouser/pointer
	that uses the Analogue stick, and the R and L trigger buttons.

	Function returns true when a key is found, else false. */

int setCustomKeyRepeat(int Rate, int Hold);
/* set custom repeat rates. 
   Rate = repeat rate (default=2)
   Hold = delay for first key (default = 16) */

int p_spSetActiveGroup(int i_active_group);
int p_sp_GetActiveGroup();
	/* allows you to programmatically set the key group */

/* modifier state bits */

	/* lookup functions, mainly for UI clients */
char p_spgetKeyCharFromKeyCode(int keyCode, int modifiers);

char p_spgetKeyCodeFromKeyId(int keyId, int keyGroup);

int p_spGetKeycodeFriendlyName(int keyCode, char keyName[]);


#define P_SP_MOD_NONE 0
#define P_SP_MOD_CONTROL 1
#define P_SP_MOD_SHIFT 2
#define P_SP_MOD_ALT 4
#define P_SP_MOD_OS 8
//#define P_SP_MOD_R_OS 16

//reserved for future implementation:
//#define P_SP_MOD_SPECIAL 16

/* key group constants */
#define P_SP_KEYGROUP_DEFAULT 0
#define P_SP_KEYGROUP_NUMFN 1 
#define P_SP_KEYGROUP_CONTROL 2 

//reserved for future implementation:
//#define P_SP_KEYGROUP_CUSTOM1 3 
//#define P_SP_KEYGROUP_CUSTOM2 4 
//#define P_SP_KEYGROUP_CUSTOM3 5 

