/* 
	Particle system for the Sony PSP.
	Copyright (C) 2005 Jesper Sandberg

	
	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#ifndef _JSAPARTICLESYSTEM_
#define _JSAPARTICLESYSTEM_

#include <list>

using namespace std;

class jsaParticleSystem
{
public:
	jsaParticleSystem() {particle_buffer = NULL;};
	virtual ~jsaParticleSystem();

	virtual bool jsaParticleSystemInit(int count);
	virtual void jsaParticleSystemUpdate();
	virtual void jsaParticleSystemRender();
protected:

	typedef struct
	{
		unsigned int	color;
		float		power;
		float		fading;
		float		x;
		float		y;
		float		z;
		float		vx;
		float		vy;
		float		vz;
		float		gx;
		float		gy;
		float		gz;
		float		mass;
	} jsaParticle;

	typedef struct
	{
		float	u,v;
		float	x,y,z;
	} jsaParticleVertex;

protected:
	int		m_particle_count;
	jsaParticle	*particle_buffer;

	void jsaParticleResetParticle(jsaParticle *particle);

};

#endif
