/*
	PSP - Particle system
	Copyright (C) 2006  Jesper Sandberg

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

#include <math.h>
#include <stdlib.h>
#include "jsaParticle.h"
/*
	Update the forces on each particle
*/
void CalculateForces(particle_str *p,int np, particlephys_str phys, particlespring_str *s,int ns)
{
	int i,p1,p2;
	vector_str down = {0.0,0.0,-1.0};
	vector_str zero = {0.0,0.0,0.0};
	vector_str f;
	float len,dx,dy,dz;

	for (i=0;i<np;i++)
	{
		p[i].f = zero;
		if (p[i].fixed)
			continue;

		/* Gravitation */
		p[i].f.x += phys.gravitational * p[i].m * down.x;
		p[i].f.y += phys.gravitational * p[i].m * down.y;
		p[i].f.z += phys.gravitational * p[i].m * down.z;

		/* Viscous drag */
		p[i].f.x -= phys.viscousdrag * p[i].v.x;
		p[i].f.y -= phys.viscousdrag * p[i].v.y;
		p[i].f.z -= phys.viscousdrag * p[i].v.z;
	}

	/* Handle the spring interaction */
	for (i=0;i<ns;i++)
	{
		p1 = s[i].from;
		p2 = s[i].to;
		dx = p[p1].p.x - p[p2].p.x;
		dy = p[p1].p.y - p[p2].p.y;
		dz = p[p1].p.z - p[p2].p.z;
		len = sqrt(dx*dx + dy*dy + dz*dz);
		f.x  = s[i].springconstant  * (len - s[i].restlength);
		f.x += s[i].dampingconstant * (p[p1].v.x - p[p2].v.x) * dx / len;
		f.x *= - dx / len;
		f.y  = s[i].springconstant  * (len - s[i].restlength);
		f.y += s[i].dampingconstant * (p[p1].v.y - p[p2].v.y) * dy / len;
		f.y *= - dy / len;
		f.z  = s[i].springconstant  * (len - s[i].restlength);
		f.z += s[i].dampingconstant * (p[p1].v.z - p[p2].v.z) * dz / len;
		f.z *= - dz / len;
		if (!p[p1].fixed)
		{
			p[p1].f.x += f.x;
			p[p1].f.y += f.y;
			p[p1].f.z += f.z;
		}
		if (!p[p2].fixed)
		{
			p[p2].f.x -= f.x;
			p[p2].f.y -= f.y;
			p[p2].f.z -= f.z;
		}
	}
}

/*
	Perform one step of the solver
*/
void UpdateParticles(particle_str *p,int np, particlephys_str phys, particlespring_str *s,int ns, float dt)
{
	int i;
	particle_str *ptmp;
	particlederivatives_str *deriv;

	deriv = (particlederivatives_str *)malloc(np * sizeof(particlederivatives_str));

	CalculateForces(p,np,phys,s,ns);
	CalculateDerivatives(p,np,deriv);
	ptmp = (particle_str *)malloc(np * sizeof(particle_str));
	for (i=0;i<np;i++)
	{
		ptmp[i] = p[i];
		ptmp[i].p.x += deriv[i].dpdt.x * dt / 2;
		ptmp[i].p.y += deriv[i].dpdt.y * dt / 2;
		ptmp[i].p.z += deriv[i].dpdt.z * dt / 2;
		ptmp[i].p.x += deriv[i].dvdt.x * dt / 2;
		ptmp[i].p.y += deriv[i].dvdt.y * dt / 2;
		ptmp[i].p.z += deriv[i].dvdt.z * dt / 2;
	}
	CalculateForces(ptmp,np,phys,s,ns);
	CalculateDerivatives(ptmp,np,deriv);
	for (i=0;i<np;i++)
	{
		p[i].p.x += deriv[i].dpdt.x * dt;
		p[i].p.y += deriv[i].dpdt.y * dt;
		p[i].p.z += deriv[i].dpdt.z * dt;
		p[i].v.x += deriv[i].dvdt.x * dt;
		p[i].v.y += deriv[i].dvdt.y * dt;
		p[i].v.z += deriv[i].dvdt.z * dt;
	}
	free(ptmp);

	free(deriv);
}

/*
	Calculate the derivatives
	dp/dt = v
	dv/dt = f / m
*/
void CalculateDerivatives(particle_str *p,int np, particlederivatives_str *deriv)
{
	int i;

	for (i=0;i<np;i++) {
		deriv[i].dpdt.x = p[i].v.x;
		deriv[i].dpdt.y = p[i].v.y;
		deriv[i].dpdt.z = p[i].v.z;
		deriv[i].dvdt.x = p[i].f.x / p[i].m;
		deriv[i].dvdt.y = p[i].f.y / p[i].m;
		deriv[i].dvdt.z = p[i].f.z / p[i].m;
	}
}
