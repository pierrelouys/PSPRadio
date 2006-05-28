#include "dlib/dlib.h"
#include "dlib/render.h"
#include "dlib/guibit.h"
#include "dlib/guibits/textArea.h"
#include "dlib/guibits/wifiSelector.h"

#include "gui/chatInput.h"
#include "gui/chatSelector.h"
#include "gui/menuPopup.h"
#include "gui/accountCreator.h"
#include "dlib/util.h"

#include "irc.h"
#include "bitlbee.h"
#include "gui/accountsStatus.h"
#include <iostream>
#include <fstream>

#ifdef PSP
	#include <pspkernel.h>
	#include <pspsdk.h>
	#include <pspnet.h>
	#include <pspnet_inet.h>
	#include <pspnet_apctl.h>
	#include <psputility_netparam.h>
	
	#ifdef PSPRADIOPLUGIN
		#include <pthread.h>
		#include <PSPRadio_Exports.h>
		#include <APP_Exports.h>
		#include <Common.h>
		PSP_MODULE_INFO("APP_afkim", 0, 1, 1);
		PSP_HEAP_SIZE_KB(1024*4);
	#else //NOT PSPRADIOPLUGIN
		PSP_MODULE_INFO("afkim", 0x1000, 1, 1);
		PSP_MAIN_THREAD_ATTR(0);
	#endif //PSPRADIOPLUGIN
#endif //PSP

using namespace std;

//GLOBAL IN INPUTABLE.h
	vector<inputable*> inputs;


#ifdef PSP
	#ifndef PSPRADIOPLUGIN
		/* Exit callback */
		int exit_callback(int arg1, int arg2, void *common)
		{
			pspSdkInetTerm();
			sceNetApctlDisconnect();
			sceNetApctlTerm();
			
			sceKernelExitGame();
			return 0;
		}
		
		/* Callback thread */
		int CallbackThread(SceSize args, void *argp)
		{
			int cbid;
		
			cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
			sceKernelRegisterExitCallback(cbid);
			sceKernelSleepThreadCB();
		
			return 0;
		}
		
		/* Sets up the callback thread and returns its thread id */
		int SetupCallbacks(void)
		{
			int thid = 0;
		
			thid = sceKernelCreateThread("update_thread", CallbackThread,
							0x11, 0xFA0, PSP_THREAD_ATTR_USER, 0);
			if(thid >= 0)
			{
				sceKernelStartThread(thid, 0, 0);
			}
		
			return thid;
		}
		
		PspDebugStackTrace st[3];
		
		/* Example custom exception handler */
		void MyExceptionHandler(PspDebugRegBlock *regs)
		{
			/* Do normal initial dump, setup screen etc */
			pspDebugScreenInit();
		
			/* I always felt BSODs were more interesting that white on black */
			pspDebugScreenSetBackColor(0x00FF0000);
			pspDebugScreenSetTextColor(0xFFFFFFFF);
			pspDebugScreenClear();
		
			pspDebugScreenPrintf("Exception Details:\n");
			pspDebugDumpException(regs);
			pspDebugScreenPrintf("\nStack Trace:\n");
			
			int size = pspDebugGetStackTrace2(regs, st, 3);
			int a;
			for (a = 0; a < size; a++)
			{
				printf("%i)  %x | %x\n", a, st[a].call_addr, st[a].func_addr);
			}
			sceKernelDelayThread(10*1000*1000);	// 10sec
		}
	#endif //PSPRADIOPLUGIN
#else //NOT PSP
#define userConnectToWifi(disconnect) 1
#endif 

#ifdef PSP
	int userMain(SceSize args, void *argp)
#else //NOT PSP
	int userMain()
#endif
{
	#ifdef PSPRADIOPLUGIN
		PSPRadioExport_RequestExclusiveAccess(PLUGIN_APP);
		char* cwd = (char*)malloc(MAXPATHLEN);
		if (cwd)
		{
			getcwd(cwd, MAXPATHLEN);
			strcat(cwd, "/APP_afkim");
			chdir(cwd);
		}
	#endif //PSPRADIOPLUGIN	
	
	//Init stuff, create base gui stuff
	renderInit();
	
	SDL_Joystick *joystick;
	joystick = SDL_JoystickOpen(0);
	guiBit abit = guiBit("./pics/bg.png");
	addGuiBit(&abit);
	renderGui();
//	SDL_Delay(2000);
	
	///load the wifi and get the user to connect
	#ifndef PSPRADIOPLUGIN
		#ifdef PSP
			int err;
			if((err = pspSdkInetInit()))
			{
				//HACK TODO OMG NO
		//		sprintf(tmpBuffer, "ERROR: could not initialise the network %08X\r\n", err);
		//		renderMain(tmpBuffer, COLOR_RED);
				exit(0);
			}
		#endif // PSP	
		
		//create wifi object.
		wifiSelector* ws = new wifiSelector(2, 8, "pics/selected.png");
		ws->setIndexVal(0);
		ws->inputableActivate();
		addGuiBit(ws);
		//loop on input
		cout << "AAA" << endl;
		while (1)
		{
			SDL_JoystickUpdate();
			int newInput = ws->takeInput(joystick);
			if (newInput != 0)
				break;
			SDL_Delay(10);
			renderGui();
		}
		ws->inputableDeactivate();
		removeGuiBit(ws);
		delete ws;
	#endif //PSPRADIOPLUGIN
	
	
	
	
	guiBit selectedText = guiBit("./pics/current.png");
//	vector<inputable*> inputs;
	int currentInput = 0;
	chatSelector* cs = new chatSelector(385,12, "./pics/select_chat.png"); //consts stolen from bitlbee.cc
	cs->inputableDeactivate();
	addGuiBit(cs);
	cs->setIndexVal(inputs.size());
	inputs.push_back(cs);

	accountsStatus ac = accountsStatus();
	ac.moveTo(480-5, 272-34);
	bitlbeeCallback* ic = bitlbeeCallback::getBee();
	ic->setBitlbeeAccountChangeCallback((bitlbeeAccountChangeCallback*)&ac);
	addGuiBit((guiBit*)&ac);

	inputable* cci = new chatInput(79, 2, 234, TEXT_NORMAL_COLOR);
	cci->inputableDeactivate();
	addGuiBit(cci);
	cci->setIndexVal(inputs.size());
	inputs.push_back(cci);
	
	//Try load the bitlbee settings from file.
	ifstream infile("./bitlbee.cfg");
	if (!infile.is_open()) //no settings file, run the first time setup thing
	{
		ic->mainTextA->setAlign(ALIGN_TOP);

		while (1)
		{
			cout << "No accounts file, need to register..." << endl;
			accountCreator* creator = new accountCreator(BAT_BITLBEE, unicodeClean(
			"No bitlbee account found, Lets create one.\n"
			"AFKIM uses the im.bitlbee.org server.\n"
			"Thanks bitlbee crew :)"), 1);//new menuPopup();
			creator->setIndexVal(0);
			creator->inputableActivate();
			renderGui();
			int at = 0;
			while (at == 0) //loops untill the menu is done menuing
			{
				SDL_JoystickUpdate();
				at = creator->takeInput(joystick);
	//			cout << "AT:" << at << endl;
				SDL_Delay(10);
				ic->poll();
				renderGui();
			}
			//create bitlbee account.
			string nUsername = unUnicode(creator->username->getText());
			string nPassword = unUnicode(creator->password->getText());
			//clear the text
			ic->mainTextA->getText()->reset();
			ic->mainTextA->clearArea();
			ic->registrationConnect("im.bitlbee.org" , 6667, nUsername, nPassword);
			
			removeGuiBit(creator);
			delete creator;
			
			cout << "STATUS:" << ic->status << endl;
			if (ic->status == BB_FAIL)
			{
				cout << "Failed to create account, possibly taken" << endl;
				continue;
			}
			else
			{
				cout << "Bitlbee account created! :)" << endl;
				ofstream outfile("./bitlbee.cfg");
				outfile << "im.bitlbee.org" << endl << nUsername << endl << nPassword << endl;
				outfile.close();
				
				break;
			}
		}
		ic->mainTextA->setAlign(ALIGN_BOTTOM);
	}
	else	//settings file exists, assume it is good and fire up the interface
	{
		cout << "Loading..." << endl;
		
		string server, user, password;
		infile >> server >> user >> password;
		
		ic->doConnect(server , 6667, user, password);
	}
	
	if (ic->status == BB_FAIL)
	{
		cout << "Failed to login to Bitlbee :(" << endl;
		ic->doDisconnect();
		#ifdef PSPRADIOPLUGIN
			PSPRadioExport_PluginExits(PLUGIN_APP); /** Notify PSPRadio, so it can unload the plugin */
		#endif //PSPRADIOPLUGIN
		return 0;
	}
	else if (ic->status == BB_IDENTIFIED)
	{
		cout << "Logged into Bitlbee!" << endl;
	}
	ic->setBitlbeeContactChangeCallback(cs);
	
	renderGui();
	ic->signin(-1);
	
	//move to pos of 0
	selectedText.moveTo(378,0);
	addGuiBit(&selectedText);

	//gotta add this last!
	int positionOfMenu = inputs.size();
	menuPopup* mmpop = new menuPopup();
	inputs.push_back(mmpop);
	mmpop->setIndexVal(positionOfMenu);
	addGuiBit(mmpop);
	while (1)
	{
		SDL_JoystickUpdate();
		
		if (PRESSING_START(joystick) && PRESSING_SELECT(joystick)) { cout << "BANG" << endl; }
		
		int newInput = inputs[currentInput]->takeInput(joystick);
		
		if (newInput == SWITCH_QUIT)
		{
			//Quiting
			break;
		}
		else if (newInput != currentInput)
		{
			cout << "Switch from " << currentInput << " -> " << newInput << endl;
			if (newInput == positionOfMenu && currentInput < positionOfMenu)
			{
				mmpop->changeReturnVal = currentInput;
			}
			inputs[currentInput]->inputableDeactivate();
			if (currentInput > positionOfMenu)
			{
				cout << "Removing one from inputs" << endl;
				delete inputs[currentInput];
				inputs.pop_back();
			}
			currentInput = newInput;
			inputs[currentInput]->inputableActivate();
			
			if (currentInput == 0)
			{
				selectedText.moveTo(378,0);
			}
			else if (currentInput == 1)
			{
				selectedText.moveTo(150,0);
			}
			cs->dirty = true;
		}
		
		ic->poll();
		SDL_Delay(10);
		renderGui();
	}
	renderGui();
//	SDL_Delay(5000);

	ic->doDisconnect();
	
	#ifdef PSPRADIOPLUGIN
		PSPRadioExport_PluginExits(PLUGIN_APP); /** Notify PSPRadio, so it can unload the plugin */
	#endif //PSPRADIOPLUGIN
	return EXIT_SUCCESS;
}

extern "C" {
	extern void _init(void); 
}

#ifdef PSPRADIOPLUGIN

/** Plugin code */
int ModuleStartAPP()
{
	pthread_t pthid;
	pthread_attr_t pthattr;
	struct sched_param shdparam;

	sleep(1);
	
	SceSize am = sceKernelTotalFreeMemSize();
	ModuleLog(LOG_INFO, "ModuleStartApp(): Available memory: %dbytes (%dKB or %dMB)", am, am/1024, am/1024/1024);
	
	pthread_attr_init(&pthattr);
	shdparam.sched_policy = SCHED_OTHER;
	shdparam.sched_priority = 45;
	pthread_attr_setschedparam(&pthattr, &shdparam);
	pthread_create(&pthid, &pthattr, (void(*)(void*))userMain, NULL);
	
	return 0;
}

int ModuleContinueApp()
{
	//TODO
	return 0;
}



#else //NOT PSPRADIOPLUGIN

/* Simple thread */
int main(int argc, char **argv)
{
#ifdef PSP
	SceUID thid;

	SetupCallbacks();
	if (sceKernelDevkitVersion() >= 0x02000010)	//fix up stuff in 2.00
		_init();

	pspDebugScreenInit();
	if(pspSdkLoadInetModules() < 0)
	{
		printf("Error, could not load inet modules\n");
		sceKernelSleepThread();
	}
	
	pspDebugInstallErrorHandler(MyExceptionHandler);

	/* Create a user thread to do the real work */
	thid = sceKernelCreateThread("userMain", userMain, 0x18, 0x10000, PSP_THREAD_ATTR_USER, NULL);
	if(thid < 0)
	{
		printf("Error, could not create thread\n");
		sceKernelSleepThread();
	}
	sceKernelStartThread(thid, 0, NULL);
	sceKernelExitDeleteThread(0);
#else //PSP
	userMain();
#endif
	return 0;
}

#endif //PSPRADIOPLUGIN
