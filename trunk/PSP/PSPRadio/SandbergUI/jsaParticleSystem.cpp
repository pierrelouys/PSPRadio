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
			particle_buffer[i].x	+= particle_buffer[i].vx;
			particle_buffer[i].y	+= particle_buffer[i].vy;
			particle_buffer[i].z	+= particle_buffer[i].vz;
			particle_buffer[i].vx	+= particle_buffer[i].gx;
			particle_buffer[i].vy	+= particle_buffer[i].gy;
			particle_buffer[i].vz	+= particle_buffer[i].gz;
		}
    	}
}

void jsaParticleSystem::jsaParticleResetParticle(jsaParticle *particle)
{
	particle->power	= 1.0f;
	particle->mass	= float((rand()%50))/2000.0+0.01;
	particle->color	= 0x44FFFFFF;
	particle->x	= 0.0f;
	particle->y	= 0.0f;
	particle->z	= 0.0f;
	particle->vx	= float((rand()%50)-25.0f)/2000.0;
	particle->vy	= float((rand()%50)-25.0f)/2000.0;
	particle->vz	= float((rand()%50)-25.0f)/2000.0;
	particle->gx	= 0.0f;
	particle->gy	= -float(rand()%10)/8000.0f+0.001;
	particle->gz	= 0.0f;
}

void jsaParticleSystem::jsaParticleSystemRender()
{
	for (int i = 0; i < m_particle_count ;  i++)
	{
		sceGuColor(particle_buffer[i].color);

		jsaParticleVertex* l_vertices = (jsaParticleVertex*)sceGuGetMemory(2 * 3 * sizeof(jsaParticleVertex));

		l_vertices[0].x = particle_buffer[i].x - 0.2f;
		l_vertices[0].y = particle_buffer[i].y - 0.2f;
		l_vertices[0].z = particle_buffer[i].z;
		l_vertices[0].u = 0;
		l_vertices[0].v = 0;

		l_vertices[1].x = particle_buffer[i].x - 0.2f;
		l_vertices[1].y = particle_buffer[i].y + 0.2f;
		l_vertices[1].z = particle_buffer[i].z;
		l_vertices[1].u = 0;
		l_vertices[1].v = 1;

		l_vertices[2].x = particle_buffer[i].x + 0.2f;
		l_vertices[2].y = particle_buffer[i].y + 0.2f;
		l_vertices[2].z = particle_buffer[i].z;
		l_vertices[2].u = 1;
		l_vertices[2].v = 1;

		l_vertices[3].x = particle_buffer[i].x - 0.2f;
		l_vertices[3].y = particle_buffer[i].y - 0.2f;
		l_vertices[3].z = particle_buffer[i].z;
		l_vertices[3].u = 0;
		l_vertices[3].v = 0;

		l_vertices[4].x = particle_buffer[i].x + 0.2f;
		l_vertices[4].y = particle_buffer[i].y + 0.2f;
		l_vertices[4].z = particle_buffer[i].z;
		l_vertices[4].u = 1;
		l_vertices[4].v = 1;

		l_vertices[5].x = particle_buffer[i].x + 0.2f;
		l_vertices[5].y = particle_buffer[i].y - 0.2f;
		l_vertices[5].z = particle_buffer[i].z;
		l_vertices[5].u = 1;
		l_vertices[5].v = 0;

		sceGumDrawArray(GU_TRIANGLES,GU_TEXTURE_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_3D,2*3,0,l_vertices);
    	}
}
