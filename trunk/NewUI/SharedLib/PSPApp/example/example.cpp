/* 
   AudioLib Sample

   Demonstrates how to get sound working with minimal effort.

   Based on sdktest sample from pspsdk
*/

#include <PSPApp.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

/* Define the module info section */
PSP_MODULE_INFO("AUDIOLIBDEMO", 0, 1, 1);
/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

const float PI = 3.1415926535897932f;
const int sampleRate = 44100;
float frequency = 440.0f;
float time = 0;
int function = 0;

typedef struct {
        short l, r;
} sample_t;




class myPSPApp : public CPSPApp
{
public:
	int Setup()
	{
		EnableAudio();
		printf("Press up and down to select frequency\nPress CROSS/SQUARE/TRIANGLE/CIRCLE to change function\n");
		
		return 0;
	}

	void OnButtonPressed(int iButtonMask)
	{
		if (iButtonMask & PSP_CTRL_CROSS)
		{
			function = 0;
		}
		else if (iButtonMask & PSP_CTRL_SQUARE)
		{
			function = 1;
		}
		else if (iButtonMask & PSP_CTRL_TRIANGLE)
		{
			function = 2;
		}
		else if (iButtonMask & PSP_CTRL_CIRCLE)
		{
			function = 3;
		}
	};
	
	void OnAnalogueStickChange(int Lx, int Ly)
	{
		/* Read the analog stick and adjust the frequency */
		const int zones[6] = {30, 70, 100, 112, 125, 130};
		const float response[6] = {0.0f, 0.1f, 0.5f, 1.0f, 4.0f, 8.0f};
		const float minFreq = 32.0f;
		const float maxFreq = 7040.0f;
		float direction;
		int i, v;
		
		v = Ly - 128;
		if (v < 0) {
	   	        direction = 1.0f;
			v = -v;
		} else {
		        direction = -1.0f;
		}
	
		for (i = 0; i < 6; i++) {
		        if (v < zones[i]) {
			          frequency += response[i] * direction;
				  break;
		        }
		}
	
		if (frequency < minFreq) {
		        frequency = minFreq;
		} else if (frequency > maxFreq) {
		        frequency = maxFreq;
		}
		
		Display();

	};

	void Display()
	{
		pspDebugScreenSetXY(0,5);
		printf("freq = %.2f   \n", frequency);
		switch(function) {
		case 0:
		  printf("sine wave\n");
		  break;
		case 1:
		  printf("square wave\n");
		  break;
		case 2:
		  printf("triangle wave\n");
		  break;
		}
	}

//	void OnVBlank()
//	{
//
//	}
	
//	/** test */
//	int OnAppExit(int arg1, int arg2, void *common)
//	{
//		sceKernelExitGame();
//		return 0;
//	}
	
	
	static float currentFunction(const float time) 
	{
	        double x;
		float t = modf(time / (2 * PI), &x);
	
	        switch(function) {
		case 0: // SINE
		        return sinf(time);
		case 1: // SQUARE
		        if (t < 0.5f) {
		                return -0.2f;
		        } else {
		                return 0.2f;
		        }
		case 2: // TRIANGLE
		        if (t < 0.5f) {
		                return t * 2.0f - 0.5f;
		        } else {
		                return 0.5f - (t - 0.5f) * 2.0f;
		        }
		default:
	 	        return 0.0f;
	        }
	};
	
	/* This function gets called by pspaudiolib every time the
    audio buffer needs to be filled. The sample format is
    16-bit, stereo. */
	void OnAudioBufferEmpty(void* buf, unsigned int length) 
	{
	        const float sampleLength = 1.0f / sampleRate;
		const float scaleFactor = SHRT_MAX - 1.0f;
	        static float freq0 = 440.0f;
	       	sample_t* ubuf = (sample_t*) buf;
		int i;
		
		if (frequency != freq0) {
		        time *= (freq0 / frequency);
		}
		for (i = 0; i < (int)length; i++) {
		        short s = (short) (scaleFactor * currentFunction(2.0f * PI * frequency * time));
			ubuf[i].l = s;
			ubuf[i].r = s;
			time += sampleLength;
		}
		if (time * frequency > 1.0f) {
		        double d;
			time = modf(time * frequency, &d) / frequency;
		}
		freq0 = frequency;
	};


};// PSPApp;




int main(void) 
{
	myPSPApp *PSPApp  = new myPSPApp();
	PSPApp->Setup();
	PSPApp->Run();
	
	return 0;
}

