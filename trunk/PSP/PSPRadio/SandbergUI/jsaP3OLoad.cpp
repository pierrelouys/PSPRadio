/* 
	P3O Loader for the Sony PSP.
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
#include <malloc.h>
#include <Logging.h>

#include "jsaP3OLoad.h"


int jsaP3OLoad::jsaP3OLoadFile(char *objectfile, jsaP3OLoad::jsaP3OInfo *object)
{
	int		return_value = JSAP3O_ERROR_LOAD;
	FILE		*fhandle;
	jsaP3OHeader	header;

	fhandle = fopen(objectfile, "r");
	if (fhandle != NULL);
	{
		int bytes;
		int vsize, fsize;

		bytes = fread(&header, 1, sizeof(jsaP3OHeader), fhandle);
		vsize = header.vertices * sizeof(jsaP3OVertex);
		fsize = header.faces * sizeof(jsaP3OFace);
		object->vertice_count	= header.vertices;
		object->face_count	= header.faces;

		if (bytes == sizeof(jsaP3OHeader))
		{
			object->vertices = (jsaP3OVertex *)memalign(16, vsize);
			object->faces = (jsaP3OFace *)memalign(16, fsize);
			if (object->vertices && object->faces)
			{
				bytes = fread(object->vertices, 1, vsize, fhandle);
				if (bytes == vsize)
				{
					bytes = fread(object->faces, 1, fsize, fhandle);
					if (bytes == fsize)
					{
						return_value = JSAP3O_ERROR_OK;
					}
					else
					{
						free(object->faces);
						object->faces = NULL;
						free(object->vertices);
						object->vertices = NULL;
					}
				}
				else
				{
					free(object->faces);
					object->faces = NULL;
					free(object->vertices);
					object->vertices = NULL;
				}
			}
			else
			{
				if (object->vertices != NULL)
				{
					free(object->vertices);
					object->vertices = NULL;
				}
				if (object->faces != NULL)
				{
					free(object->faces);
					object->faces = NULL;
				}
				return_value = JSAP3O_ERROR_MEMORY;
			}
		}
		fclose(fhandle);
	}
	sceKernelDcacheWritebackAll();

	return return_value;
}
