#include "accountsStatus.h"
#include <SDL/SDL_image.h>
#include "../dlib/render.h"
#include <iostream>
#include "../dlib/util.h"

#define SPACE_BETWEEN_IMAGES 2
strSurfP::strSurfP(const string &nName)
{
	name = nName;
	surface = IMG_Load(("icons/" + name + ".png").c_str());
	if (surface == NULL) {
		printf(("Unable to load image! " + name + "\n").c_str());
		return;
	}
}

void accountsStatus::addPic(const string &nName)
{
	surfaces.push_back(strSurfP(nName));
}

SDL_Surface* accountsStatus::getPic(const string &name)
{
	for (unsigned int a = 0; a < surfaces.size(); a++)
	{
		if (!nocase_cmp(surfaces[a].name, name))//same
			return surfaces[a].surface;
	}
	return NULL;
}

accountsStatus::accountsStatus()
{
	cout << "accountsStatus()" << endl;
	dirty = false;
	
	//Load images
	addPic("toc");
	addPic("toc_bw");
	addPic("icq");
	addPic("icq_bw");
	addPic("jabber");
	addPic("jabber_bw");
	addPic("msn");
	addPic("msn_bw");
	addPic("yahoo");
	addPic("yahoo_bw");
	
	offset = screen_rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = 0;
	rect.h = 0;
	
	pixels = NULL;
}

accountsStatus::~accountsStatus()
{
	for(int a = 0; a < surfaces.size(); a++)
	{
		SDL_FreeSurface(surfaces[a].surface);
		surfaces[a].surface = NULL;
	}
	surfaces.clear();
}

void accountsStatus::draw()
{
	dirty = false;
	
	SDL_Rect backup_offset = offset;
	for (unsigned int a = 0; a < accountList.size(); a++)
	{
		string accountType = accountList[a].type;
		if (accountList[a].status != BA_ONLINE)
			accountType += "_bw";
		SDL_Surface* surf = getPic(accountType);
		if (surf == NULL) 
		{
			cout << "CANT FIND ACCOUNT TYPE :" << accountType << ":" << endl;
			continue;
		}
		offset.x -= (surf->w - 8);
		
		rect.x = 0;
		rect.y = 0;
		rect.w = surf->w;
		rect.h = surf->h;
		
		SDL_BlitSurface(surf, &rect, screen, &offset);
		offset.x -= SPACE_BETWEEN_IMAGES;
	}
	offset = backup_offset;
}

bool accountsStatus::needsRedraw()
{
	return dirty;
}

void accountsStatus::notifyAccountChange(const vector < bitlbeeAccount > &nAccountList)
{
	cout << "accountsStatus::notifyAccountChange()" << endl;
	accountList = nAccountList;
	dirty = true;
}
