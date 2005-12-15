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

#include <stdio.h>
#include <Logging.h>
#include <malloc.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspge.h>

#include <jsaParticleSystem.h>

jsaParticleSystem::~jsaParticleSystem()
{
	if (particle_buffer == NULL)
	{
		free(particle_buffer);
	}
}

bool jsaParticleSystem::jsaParticleSystemInit(int count)
{
	bool ret_value = false;

	m_particle_count = count;

	if (particle_buffer == NULL)
	{
		free(particle_buffer);
	}

	particle_buffer = (jsaParticle *) memalign(16, count * sizeof(jsaParticle));

	if (particle_buffer)
	{
		for (int i = 0 ; i < count ; i++)
		{
			jsaParticleResetParticle(&particle_buffer[i]);
		}
		ret_value = true;
	}
	return ret_value;
}

void jsaParticleSystem::jsaParticleSystemUpdate()
{
	for (int i = 0; i < m_particle_count ; i++)
	{
		particle_buffer[i].power -= particle_buffer[i].mass;

		if (particle_buffer[i].power < 0.0f)
		{
			jsaParticleResetParticle(&particle_buffer[i]);
		}
		else
		{
			unsigned char	a = (particle_buffer[i].orig_color >> 24) & 0xFF;
			particle_buffer[i].x	+= particle_buffer[i].vx;
			particle_buffer[i].y	+= particle_buffer[i].vy;
			particle_buffer[i].z	+= particle_buffer[i].vz;
			particle_buffer[i].vx	+= particle_buffer[i].gx;
			particle_buffer[i].vy	+= particle_buffer[i].gy;
			particle_buffer[i].vz	+= particle_buffer[i].gz;
			particle_buffer[i].color	= particle_buffer[i].orig_color & 0x00FFFFFF;
			particle_buffer[i].color	|= (unsigned long)((unsigned long)(particle_buffer[i].power * a) << 24);
		}
    	}
}

void jsaParticleSystem::jsaParticleResetParticle(jsaParticle *particle)
{
	particle->power	= 1.0f;
	particle->mass	= float((rand()%50))/5000.0 + 0.01;
	particle->color	= 0xFFFFFFFF;
	particle->orig_color	= 0xFFFFFFFF;
	particle->x	= 240.0f;
	particle->y	= 160.0f;
	particle->z	= 0.0f;
	particle->vx	= float((rand()%50)-25.0f)/50.0;
	particle->vy	= float((rand()%50)-25.0f)/50.0;
	particle->vz	= float((rand()%50)-25.0f)/50.0;
	particle->gx	= 0.0f;
	particle->gy	= -(float((rand()%50))/5000.0 + 0.005);
	particle->gz	= 0.0f;
}

void jsaParticleSystem::jsaParticleSystemRender()
{
	jsaParticleVertex* l_vertices = (jsaParticleVertex*)sceGuGetMemory(m_particle_count * 2 * sizeof(jsaParticleVertex));

	for (int i = 0; i < m_particle_count ;  i++)
	{
		jsaParticleVertex *index = &l_vertices[i<<1]; 

		index[0].u = 0;
		index[0].v = 0;
		index[0].color = particle_buffer[i].color;
		index[0].x = particle_buffer[i].x - 9.9f;
		index[0].y = particle_buffer[i].y - 9.9f;
		index[0].z = particle_buffer[i].z;

		index[1].u = 32;
		index[1].v = 32;
		index[1].color = particle_buffer[i].color;
		index[1].x = particle_buffer[i].x + 9.9f;
		index[1].y = particle_buffer[i].y + 9.9f;
		index[1].z = particle_buffer[i].z;
    	}
	sceGuDrawArray(GU_SPRITES, GU_COLOR_8888 | GU_TEXTURE_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_2D,2 * m_particle_count,0,l_vertices);
}
