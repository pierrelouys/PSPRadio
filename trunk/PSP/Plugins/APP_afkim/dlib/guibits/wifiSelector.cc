#include "wifiSelector.h"
#include "../render.h"
#include <iostream>
#include "../util.h"

#ifdef PSP
#include <pspsdk.h>
#include <pspnet_apctl.h>
#include <psputility_netparam.h>
#endif

wifiSelector::wifiSelector(const unsigned int &targetX, const unsigned int &targetY, const string &image)
{
	currentChoice = 0;

	infoText = textBlock(62, 21);
	text = new textArea(62, 21, &infoText);
	text->moveTo(targetX+10,targetY);
	text->setAlign(ALIGN_TOP);
	
	logText = textBlock(62, 10);
	text2 = new textArea(62, 10, &logText);
	text2->moveTo(targetX, 112);
	text2->setAlign(ALIGN_BOTTOM);
	text2->addText(unicodeClean("Choose a wifi connection to connect to\n"), TEXT_NORMAL_COLOR);

	pick_count = 0;
	#ifdef PSP
	int iNetIndex;
	for (iNetIndex = 1; iNetIndex < 100; iNetIndex++) // skip the 0th connection
	{
		if (sceUtilityCheckNetParam(iNetIndex) != 0)
			continue;  // this one is no good
		sceUtilityGetNetParam(iNetIndex, 0, (netData*)choices[pick_count].name);
		choices[pick_count].index = iNetIndex;
		pick_count++;
		if (pick_count >= 10)
			break;  // no more room
	}
	#else
	pick_count = 2;
	choices[0].index = 1;
	choices[1].index = 1;
	sprintf(choices[0].name, "connection 1");
	sprintf(choices[1].name, "connection 2");
	#endif
	
	for (int a = 0; a < pick_count; a++)
	{
		text->addText(unicodeClean(string(choices[a].name)+"\n"), TEXT_NORMAL_COLOR);
	}
	
	if (pick_count == 0)
	{
		text2->addText(unicodeClean("No connections\nPlease try Network Settings\n"), TEXT_ERROR_COLOR);
		text2->draw();
		SDL_UpdateRect(screen, 0, 0, 0, 0);
		SDL_Delay(10000); // 10sec to read before exit
		exit(0);
	}
	
	init(targetX, targetY-1, 10, 0, image);
}

wifiSelector::~wifiSelector()
{
	delete text;
	delete text2;
}

string wifiSelector::getInputKey() const
{
	return "wifiSelector";
}

string wifiSelector::fixSelected()
{
	//wrap in either direction
	if (selected < 0)
		selected += pick_count;
	if (selected >= (int)pick_count)
		selected = 0;
	
	return getInputKey();
}


string wifiSelector::pressCross()
{
	cout << "X" << endl;
#ifdef PSP
	int err;
	int stateLast = -1;
	
connectWifi:
	
	err = sceNetApctlConnect(choices[selected].index);
	if (err != 0)
	{
//		text2->addText(unicodeClean(string("ERROR: sceNetApctlConnect returns ") + err + string("\n")), TEXT_ERROR_COLOR);
		text2->addText(unicodeClean(string("ERROR: sceNetApctlConnect returns Error\n")), TEXT_ERROR_COLOR);
		dirty = true;
		renderGui();
		return 0;
	}

	text2->addText(unicodeClean("connecting\n"), 0xFFFFFFFF);
	dirty = true;
	renderGui();
	while (1)
	{
		int state;
		err = sceNetApctlGetState(&state);
		if (err != 0)
		{
//			text2->addText(unicodeClean(string("ERROR: sceNetApctlGetState returns ") + err + string("\n")), TEXT_ERROR_COLOR);
			text2->addText(unicodeClean(string("ERROR: sceNetApctlGetState returns Error\n")), TEXT_ERROR_COLOR);
			dirty = true;
			renderGui();
			break;
		}
		if (state != stateLast)
		{
			if (stateLast == 2 && state == 0)
			{
				text2->addText(unicodeClean("  Connecting to wifi Failed, Retrying...\n"), TEXT_ERROR_COLOR);
				dirty = true;
				renderGui();
				SDL_Delay(500);
				stateLast = state;
				goto connectWifi;
			}
			text2->addText(unicodeClean(string("  connection state ") + (char)('0'+state) + string(" of 4\n")), TEXT_ERROR_COLOR);
			dirty = true;
			renderGui();
			stateLast = state;
		}
		if (state == 4)
			break;  // connected with static IP

		// wait a little before polling again
		sceKernelDelayThread(50*1000); // 50ms
	}
#endif
	
	return "done";
}

string wifiSelector::pressSelect()
{
	return getInputKey();
}

void wifiSelector::draw()
{
	dirty = false;
	
	//update offset
	offset = targetOffset;
	offset.y += selected * itemSize;
	//draw
	SDL_BlitSurface(pixels, &rect, screen, &offset);
	
	text->draw();

	text2->draw();
}

