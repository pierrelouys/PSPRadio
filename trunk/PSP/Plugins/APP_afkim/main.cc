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
	#include <psputility.h>
	
	#ifdef PSPRADIOPLUGIN
		#include <pthread.h>
		#include <PSPRadio_Exports.h>
		#include <APP_Exports.h>
		#include <Common.h>
		PSP_MODULE_INFO("APP_afkim", 0, 1, 1);
		PSP_HEAP_SIZE_KB(1024*4);
	#else //NOT PSPRADIOPLUGIN
		PSP_MODULE_INFO("afkim", 0, 0, 1);
		PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
		PSP_HEAP_SIZE_KB(1024*20);
	#endif //PSPRADIOPLUGIN
#endif //PSP

using namespace std;

//GLOBAL IN INPUTABLE.h
	map<string, inputable*> inputs;


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
		ws->inputableActivate();
		addGuiBit(ws);
		//loop on input
		cout << "AAA" << endl;
		while (1)
		{
			SDL_JoystickUpdate();
			string newInput = ws->takeInput(joystick);
			if (newInput != "wifiSelector")
				break;
			SDL_Delay(10);
			renderGui();
		}
		ws->inputableDeactivate();
		removeGuiBit(ws);
		delete ws;
	#endif //PSPRADIOPLUGIN
	
	
	
	
	guiBit selectedText = guiBit("./pics/current.png");
	string currentInput = "";
	
	//TODO :Scope this out
	chatSelector* cs = new chatSelector(385,12, "./pics/select_chat.png"); //consts stolen from bitlbee.cc
	cs->inputableDeactivate();
	addGuiBit(cs);
	inputs[cs->getInputKey()] = cs;

	//TODO: Make this a pointer and lose it in the guibits map?
	accountsStatus ac = accountsStatus();
	ac.moveTo(480-5, 272-34);
	bitlbeeCallback* ic = bitlbeeCallback::getBee();
	ic->setBitlbeeAccountChangeCallback((bitlbeeAccountChangeCallback*)&ac);
	addGuiBit((guiBit*)&ac);

	//TODO: Scope this out
	inputable* cci = new chatInput(79, 2, 234, TEXT_NORMAL_COLOR);
	cci->inputableDeactivate();
	addGuiBit(cci);
	inputs[cci->getInputKey()] = cci;
	
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
			"Thanks bitlbee crew :)"), "done");//new menuPopup();
			creator->inputableActivate();
			renderGui();
			string at = "accountCreator";
			while (at == "accountCreator") //loops untill the menu is done menuing
			{
				SDL_JoystickUpdate();
				at = creator->takeInput(joystick);
	//			cout << "AT:" << at << endl;
				SDL_Delay(10); //FIXME: Remove all these in PSP build?
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
		cout << "Loading settings from bitlbee.cfg" << endl;
		
		string server, user, password;
		infile >> server >> user >> password;
		
		ic->doConnect(server , 6667, user, password);
	}
	
	if (ic->status == BB_FAIL)
	{
		//FIXME: This shouldn't make you quit
		cout << "Failed to login to Bitlbee :(" << endl;
		renderGui();
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

	//TODO: Scope this out
	menuPopup* mmpop = new menuPopup();
	addGuiBit(mmpop);
	inputs[mmpop->getInputKey()] = mmpop;
	
	currentInput = "chatSelector";
	mmpop->changeReturnVal = "chatSelector";
	
	while (1)
	{
		SDL_JoystickUpdate();
		
		if (PRESSING_START(joystick) && PRESSING_SELECT(joystick)) { cout << "BANG" << endl; exit(0); }
		
		string newInput = inputs[currentInput]->takeInput(joystick);
		
		if (newInput == SWITCH_QUIT)
		{
			//Quiting
			break;
		}
		else if (newInput != currentInput)
		{
			cout << "Switch from " << currentInput << " -> " << newInput << endl;
			
			//Menu popup needs to know where it was called from, these are the only possibilities
			if (currentInput == "chatInput" || currentInput == "chatSelector")
				mmpop->changeReturnVal = currentInput;
			
			inputs[currentInput]->inputableDeactivate();
			
			//accountCreators need to be deleted when they are closed
			if (currentInput == "accountCreator")
			{
				delete inputs["accountCreator"];
				inputs["accountCreator"] = NULL;
			}
			
			currentInput = newInput;
			inputs[currentInput]->inputableActivate();
			
			//Move the highlighter when the user selects either of these
			if (currentInput == "chatSelector")
				selectedText.moveTo(378,0);
			else if (currentInput == "chatInput")
				selectedText.moveTo(150,0);
			
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

	#if 0
	if(pspSdkLoadInetModules() < 0)
	{
		printf("Error, could not load inet modules\n");
		sceKernelSleepThread();
	}
	#else
	sceUtilityLoadNetModule(1);
	sceUtilityLoadNetModule(3);
	#endif
	
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
