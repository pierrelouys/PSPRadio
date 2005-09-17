/* 
	PSPApp Example 2
	Using MAD
*/
#include <PSPApp.h>
#include <PSPSound_MP3.h>
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>

/* Define the module info section */
PSP_MODULE_INFO("MADEXAMPLE", 0, 1, 1);
/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);


class myPSPApp : public CPSPApp
{
public:
	/** Setup */
	int Setup()
	{
		printf("PSPApp MAD Example...\n");
		
		CPSPSound_MP3 *MP3 = new CPSPSound_MP3();
		if (MP3)
		{
			MP3->SetFile("ms0:/believe.mp3");
			MP3->Play();
		}
	
		return 0;
	}
	
 
	void OnButtonPressed(int iButtonMask)
	{
		pspDebugScreenSetXY(0,25);
		if (iButtonMask & PSP_CTRL_CROSS)
		{
			printf ("CROSS    ");
		}
		else if (iButtonMask & PSP_CTRL_SQUARE)
		{
			printf ("SQUARE   ");
		}
		else if (iButtonMask & PSP_CTRL_TRIANGLE)
		{
			printf ("TRIANGLE ");
		}
		else if (iButtonMask & PSP_CTRL_CIRCLE)
		{
			printf ("CIRCLE   ");
		}
	};
	
};
	
/** main */
int main(void) 
{
	myPSPApp *PSPApp  = new myPSPApp();
	PSPApp->Setup();
	PSPApp->Run();
	
	return 0;
}

