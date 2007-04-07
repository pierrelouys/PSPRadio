/*
	PSPRadio / Music streaming client for the PSP. (Initial Release: Sept. 2005)
	Copyright (C) 2005  Rafael Cabezas a.k.a. Raf

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
#include <stdarg.h>
#include "vis_if.h"

extern "C" {
#include "kiss_fft.h"
#include "kiss_fftr.h"
};

float g_FreqData[2][NFREQS];

kiss_fftr_cfg cfg=NULL;
kiss_fft_scalar *tbuf;
kiss_fft_cpx *fbuf;
//float *mag2buf;
int i;
int n;
//int avgctr=0;

//int nfreqs=nfft/2+1;

int remove_dc = 0;

//int navg=20;
//float * vals=NULL;

extern int16 *g_PCMBuffer;

/* Based on kiss_fft's transform_signal() */
void do_fft_init()
{
    cfg=kiss_fftr_alloc(NFFT,0,0,0);
    tbuf=(kiss_fft_scalar*)malloc(sizeof(kiss_fft_scalar)*NFFT );
    fbuf=(kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx)*NFREQS );
//    CHECKNULL( mag2buf=(float*)malloc(sizeof(float)*NFREQS ) );

//    memset(mag2buf,0,sizeof(mag2buf)*NFREQS);

}


void do_fft_destroy()
{
    free(cfg);
    free(tbuf);
    free(fbuf);
//    free(mag2buf);
}

/* cabs/hypot implementation */
float vfpu_cabsf(float r, float i)
{
    /* float result = sqrt(r*r + i*i); */
#if 0
	ScePspFVector2 a = { r, i };
	float result;
   __asm__ volatile (
       "lv.q   C000, %1\n"
       "vdot.p S010, C000, C000\n"
       "vsqrt.s S010, S010\n"
	   "mfv %0, S010\n"
       : "=r"(result) : "m"(a));
	return result;
#else
	return (sqrtf(r*r + i*i));
#endif

}


void do_fft()
{
	/* Stereo */
	for (i=0;i<NFFT;++i) 
		tbuf[i] = g_PCMBuffer[2*i] >> 2;// + g_PCMBuffer[2*i+1];

	if (remove_dc) {
		float avg = 0;
		for (i=0;i<NFFT;++i)  avg += tbuf[i];
		avg /= NFFT;
		for (i=0;i<NFFT;++i)  tbuf[i] -= (kiss_fft_scalar)avg;
	}

	/* do FFT */
	kiss_fftr(cfg,tbuf,fbuf);

	for (i=0;i<NFREQS;++i)
	{
		//mag2buf[i] += fbuf[i].r * fbuf[i].r + fbuf[i].i * fbuf[i].i;
		g_FreqData[0][i] = vfpu_cabsf(fbuf[i].r, fbuf[i].i) * 64;
		//g_FreqData[1][i] = g_FreqData[0][i];
	}
}

#include <PSPRadio.h>
extern CPSPRadio *gPSPRadio;
void Vis_get_song_data(char *szURI, char *szTitle, char *szArtist, int buff_size)
{
	strlcpy(szURI, gPSPRadio->GetSoundObject()->GetCurrentStream()->GetMetaData()->strURI, buff_size);
	strlcpy(szTitle, gPSPRadio->GetSoundObject()->GetCurrentStream()->GetMetaData()->strTitle, buff_size);
	strlcpy(szArtist, gPSPRadio->GetSoundObject()->GetCurrentStream()->GetMetaData()->strArtist, buff_size);
}
