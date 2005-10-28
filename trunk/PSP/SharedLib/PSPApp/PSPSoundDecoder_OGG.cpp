/* 
	PSPApp C++ OO Application Framework. (Initial Release: Sept. 2005)
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
#include <list>
#include <PSPApp.h>
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ivorbisfile.h>
#include "bstdfile.h"
#include <malloc.h>
#include "PSPSoundDecoder_OGG.h"
using namespace std;

static size_t sg_lBytesReadFromStream = 0;

size_t ogg_socket_read_wrapper(void *buffer, size_t size, size_t nmemb, void *InputStreamPtr)
{
	CPSPStream *InputStream = (CPSPStream *)InputStreamPtr;
	size_t bytes_read = 0;
	
	bytes_read = InputStream->Read((u8*)buffer, size*nmemb);
	sg_lBytesReadFromStream += bytes_read;
	
	return bytes_read;
}

int ogg_socket_seek_wrapper(void *pSocket, ogg_int64_t offset, int whence)
{
	return -1;
}

int ogg_socket_close_wrapper(void *InputStreamPtr)
{
	return 0;
}

long ogg_socket_tell_wrapper(void *datasource)
{
	return sg_lBytesReadFromStream;
}

/** ---------------------------------------------------------------------------------- **/
void CPSPSoundDecoder_OGG::Initialize()
{
	Log(LOG_LOWLEVEL, "CPSPSoundDecoder_OGG Initialize"); 

	m_pInputBuffer = (unsigned char*)malloc(INPUT_BUFFER_SIZE);
	if (!m_pInputBuffer)
	{
		ReportError("Memory allocation error!\n");
		Log(LOG_ERROR, "Memory allocation error!\n");
		return;
	}

	m_lock = new CLock("OggReaderLock");

	if (false == OpenVorbis())
	{
		m_InputStream->Close();
	}
	sg_lBytesReadFromStream = 0; /** Reset number of bytes read from stream */
	m_last_section = -1;
}

bool CPSPSoundDecoder_OGG::OpenVorbis()
{
	bool success = false;
	int iRet  = -1;
	m_lock->Lock(); /** Vorbis is not thread safe */
			
	ov_callbacks ogg_callbacks;
	ogg_callbacks.read_func  = ogg_socket_read_wrapper;
	ogg_callbacks.seek_func  = ogg_socket_seek_wrapper;
	ogg_callbacks.close_func = ogg_socket_close_wrapper;
	ogg_callbacks.tell_func  = ogg_socket_tell_wrapper;
	
	iRet = ov_open_callbacks(m_InputStream, &m_vf, NULL, 0, ogg_callbacks);
	
	switch(iRet)
	{
		case 0: /** Success! */
		{
			ProcessInfo();
			ReadComments();
			pPSPSound->SendEvent(MID_NEW_METADATA_AVAILABLE);
			success = true;
		}			
		break;
		case OV_EREAD:
			ReportError("OGGError: A read from media returned an error.");
			success = false;
			break;
		case OV_ENOTVORBIS:
			ReportError("OGGError: Bitstream is not Vorbis data.");
			success = false;
			break;
		case OV_EVERSION:
			ReportError("OGGError: Vorbis version mismatch.");
			success = false;
			break;
		case OV_EBADHEADER:
			ReportError("OGGError: Invalid Vorbis bitstream header.");
			success = false;
			break;
		case OV_EFAULT:
			ReportError("Internal logic fault; indicates a bug or heap/stack corruption.");
			success = false;
			break;
	}
	m_lock->Unlock(); /** Vorbis is not thread safe */

	return success;
}
			
CPSPSoundDecoder_OGG::~CPSPSoundDecoder_OGG()
{
	Log(LOG_VERYLOW, "~CPSPSoundDecoder_OGG start");
	
	if (m_pInputBuffer)
	{
		free(m_pInputBuffer), m_pInputBuffer = NULL;
	}
	if (m_lock)
	{
		delete(m_lock), m_lock = NULL;
	}
	Log(LOG_VERYLOW, "~CPSPSoundDecoder_OGG end");
}

void CPSPSoundDecoder_OGG::ProcessInfo()
{
	vorbis_info *vi=ov_info(&m_vf,-1);
	Log(LOG_INFO, "ProcessInfo(): Bitstream is %d channel, %ldHz",vi->channels,vi->rate);
	Log(LOG_INFO, "ProcessInfo(): Decoded length: %ld samples", (long)ov_pcm_total(&m_vf,-1));
	m_InputStream->SetLength(ov_pcm_total(&m_vf,-1));
	Log(LOG_INFO, "ProcessInfo(): Encoded by: %s",ov_comment(&m_vf,-1)->vendor);
	m_InputStream->SetBitRate(vi->bitrate_nominal);
	m_InputStream->SetSampleRate(vi->rate);
	m_InputStream->SetNumberOfChannels(vi->channels);
	Log(LOG_VERYLOW, "ProcessInfo(): End.");
}

void CPSPSoundDecoder_OGG::ReadComments()
{
	char **ptr=ov_comment(&m_vf,-1)->user_comments;
	while(*ptr)
	{
		Log(LOG_INFO, "%s",*ptr);
		if (strstr(*ptr, "TITLE="))
		{
			m_InputStream->SetTitle((*ptr)+strlen("TITLE="));
		}
		else if (strstr(*ptr, "ARTIST="))
		{
			m_InputStream->SetArtist((*ptr)+strlen("ARTIST="));
		}
		++ptr;
	}
}

void CPSPSoundDecoder_OGG::CloseVorbis()
{
	ov_clear(&m_vf);
	//m_InputStream->Close();
}

size_t CPSPSoundDecoder_OGG::ReadVorbis(unsigned char *pBuffer, size_t SizeInBytes)
{
	size_t BytesRead = 0;
	long lRet = 0;
	int current_section = m_last_section;
	
	while ( (BytesRead < SizeInBytes) && (false == m_InputStream->IsEOF()) )
	{
		lRet = ov_read(&m_vf, (char *)(pBuffer+BytesRead), SizeInBytes-BytesRead, &current_section);
	
		if ((current_section != m_last_section) && (lRet != 0))
		{
			Log(LOG_LOWLEVEL, "ReadVorbis(): m_last_section changed from %d to %d. Processing Changes...(lRet=%d)", 
				m_last_section, current_section, lRet);
			ReadComments(); /** Get new stream information */
			ProcessInfo(); /** Get metadata from the stream */
			m_last_section = current_section;
			pPSPSound->SendEvent(MID_NEW_METADATA_AVAILABLE);
			//CloseVorbis();
			//m_InputStream->Close();
			//m_InputStream->Open();
			//if (m_InputStream->IsOpen())
			//	OpenVorbis();
			continue;
		}
		
		switch(lRet)
		{
			case 0:
				Log(LOG_INFO, "ReadVorbis(): OGG Stream End....Closing..");
				CloseVorbis();
				m_InputStream->Close();
				//m_InputStream->Open();
				//if (m_InputStream->IsOpen())
				//	OpenVorbis();
				break;
			case OV_HOLE:
				//indicates there was an interruption in the data.
				//(one of: garbage between pages, loss of sync followed by recapture, or a corrupt page)
				Log(LOG_LOWLEVEL, "ReadVorbis(): OGG Stream Warning: OV_HOLE (Garbage/loss of sync/corrupt page)");
				//Close();
				//m_eof = true;
				//return BytesRead;
				break;
			case OV_EBADLINK:
				//indicates that an invalid stream section was supplied to libvorbisidec, or 
				//the requested link is corrupt.
				Log(LOG_LOWLEVEL, "ReadVorbis(): OGG Stream Warning: OV_EBADLINK (Invalid stream section/corrupted link)");
				//Close();
				//m_eof = true;
				break;
			default:
				if (lRet > 0)
				{
					BytesRead += lRet;
				}
				else
				{
					Log(LOG_ERROR, "ReadVorbis(): Unknown error %d", lRet);
				}
				break;
		}
		
	}
	
	//Log(LOG_VERYLOW, "Read. (End) bitstream=%d", bitstream);
	return BytesRead;
}

bool CPSPSoundDecoder_OGG::Decode()
{
	long lFramesDecoded = 0;
	Sample *pOutputSample = (Sample *)m_pInputBuffer;
	Sample SampleL = 0, SampleR = 0;
	
	PCMFrameInSamples PCMOutputFrame;/** The output buffer holds one BUFFER */
	
	long lRet = 0;
	
	int iNumberOfChannels = m_InputStream->GetNumberOfChannels();
	
	lRet = ReadVorbis(m_pInputBuffer, 4096);
	
	if (lRet > 0)
	{
		/** Push decoded buffers */
		
		lFramesDecoded = BYTES_TO_FRAMES(lRet);
		pOutputSample = (Sample *)m_pInputBuffer;
		
		for (long Cnt = 0; Cnt < lFramesDecoded; Cnt++)
		{
			SampleL = *pOutputSample++;
			if(2 == iNumberOfChannels)
			{
				SampleR = *pOutputSample++; 
			}
			else
			{
				SampleR = SampleL;
			}
		
			PCMOutputFrame.RSample = SampleR;
			PCMOutputFrame.LSample = SampleL;
			
			m_Buffer->PushFrame(*((::Frame*)&PCMOutputFrame));
		}
	}
	
	return m_InputStream->IsEOF();
}
