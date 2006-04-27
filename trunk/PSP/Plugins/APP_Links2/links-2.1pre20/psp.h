#ifndef psp_h
	#define psp_h

	#ifndef tBoolean
	#define tBoolean int
	#define truE 1
	#define falsE 0
	#define KEY_BACKSPACE 8
	#endif
		
	extern volatile tBoolean g_PSPEnableRendering;
	extern volatile tBoolean g_PSPEnableInput;

#endif
