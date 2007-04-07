#ifndef _PSPRADIOVISIF_
#define _PSPRADIOVISIF_

	#ifdef __cplusplus
		extern "C"
		{
	#endif
	
		#define NFFT 512 /* number of samples */
		#define NFREQS (NFFT/2+1)
		
		extern float g_FreqData[2][NFREQS];
		
		void do_fft();
		void do_fft_init();
		void do_fft_destroy();

		void Vis_get_song_data(char *szURI, char *szTitle, char *szArtist, int buff_size);



	#ifdef __cplusplus
		};
	#endif


#endif
