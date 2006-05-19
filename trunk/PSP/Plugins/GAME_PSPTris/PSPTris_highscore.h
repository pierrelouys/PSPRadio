/*
	PSPTris - The game - Highscore handling
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

#ifndef _PSPTRIS_HIGHSCORE_H_
#define _PSPTRIS_HIGHSCORE_H_

typedef struct
	{
	u32		score;
	char	name[4];	/* 3 chars + zero termination */
	u32		rank;
	} highscore_str;

void PSPTris_highscore_init(char *cwd);
u32 PSPTris_highscore_check(u32 score, u32 game_type);
void PSPTris_highscore_store(highscore_str *highscore, u32 game_type);
highscore_str *PSPTris_highscore_get(u32 rank, u32 game_type);

#endif /* _PSPTRIS_HIGHSCORE_H_ */
