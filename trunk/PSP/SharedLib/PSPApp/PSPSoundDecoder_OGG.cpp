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
#include <PSPNet.h>
#include <ivorbisfile.h>
#include "bstdfile.h"
#include <malloc.h>
#include "PSPSoundDecoder_OGG.h"
using namespace std;

static size_t sg_lBytesReadFromStream = 0;

size_t ogg_socket_read_wrapper(void *ptr, size_t size, size_t nmemb, void *pSocket)
{
	size_t bytes_read = recv(*(int*)pSocket, ptr, size*nmemb, 0);
	sg_lBytesReadFromStream += bytes_read;
	
	return bytes_read;
}

int ogg_socket_seek_wrapper(void *pSocket, ogg_int64_t offset, int whence)
{
	return -1;
}

int ogg_socket_close_wrapper(void *pSocket)
{
	return sceNetInetClose(*(int*)pSocket);
}

long ogg_socket_tell_wrapper(void *datasource)
{
	return sg_lBytesReadFromStream;
}

COGGStreamReader::COGGStreamReader()
{
	sg_lBytesReadFromStream = 0; /** Reset number of bytes read from stream */
	m_last_section = -1;
	m_lock = new CLock("OggReaderLock");
	m_eof = false;
	m_iMetaDataInterval = CurrentSoundStream->GetMetaDataInterval();
	m_iRunningCountModMetadataInterval = 0;
	memset(bMetaData, 0, MAX_METADATA_SIZE);
 	memset(bPrevMetaData, 0, MAX_METADATA_SIZE);
	m_pfd = CurrentSoundStream->GetFileDescriptor();
	m_fdSocket = CurrentSoundStream->GetSocketDescriptor();
	
	int iRet  = -1;
	m_lock->Lock(); /** Vorbis is not thread safe */
	switch (CurrentSoundStream->GetType())
	{
		case CPSPSoundStream::STREAM_TYPE_FILE:
			iRet = ov_open(m_pfd, &m_vf, NULL /*char *initial*/, 0 /*long ibytes*/);
			break;
		
		case CPSPSoundStream::STREAM_TYPE_URL:
			ov_callbacks ogg_callbacks;
			ogg_callbacks.read_func  = ogg_socket_read_wrapper;
			ogg_callbacks.seek_func  = ogg_socket_seek_wrapper;
			ogg_callbacks.close_func = ogg_socket_close_wrapper;
			ogg_callbacks.tell_func  = ogg_socket_tell_wrapper;
			
			iRet = ov_open_callbacks(&m_fdSocket, &m_vf, NULL /*char *initial*/, 0 /*long ibytes*/, ogg_callbacks);
			break;
		
		default:
			break;
	}
	switch(iRet)
	{
		case 0: /** Success! */
		{
			ProcessInfo();
			ReadComments();
			pPSPSound->SendEvent(MID_NEW_METADATA_AVAILABLE);
		}			
		break;
		case OV_EREAD:
			ReportError("OGGError: A read from media returned an error.");
			break;
		case OV_ENOTVORBIS:
			ReportError("OGGError: Bitstream is not Vorbis data.");
			break;
		case OV_EVERSION:
			ReportError("OGGError: Vorbis version mismatch.");
			break;
		case OV_EBADHEADER:
			ReportError("OGGError: Invalid Vorbis bitstream header.");
			break;
		case OV_EFAULT:
			ReportError("Internal logic fault; indicates a bug or heap/stack corruption.");
			break;
	}
	m_lock->Unlock(); /** Vorbis is not thread safe */
		
	/** Only tell soundstream to close if ov_open fails.. If ov_open succeeds, then the
	 *  Stream needs to be closed with ov_clear! */
	if (iRet < 0)
	{
		CurrentSoundStream->Close();
	}
}

COGGStreamReader::~COGGStreamReader()
{
	if (m_lock)
	{
		delete(m_lock), m_lock = NULL;
	}
}

void COGGStreamReader::ProcessInfo()
{
	vorbis_info *vi=ov_info(&m_vf,-1);
	Log(LOG_INFO, "ProcessInfo(): Bitstream is %d channel, %ldHz",vi->channels,vi->rate);
	Log(LOG_INFO, "ProcessInfo(): Decoded length: %ld samples", (long)ov_pcm_total(&m_vf,-1));
	CurrentSoundStream->SetLength(ov_pcm_total(&m_vf,-1));
	Log(LOG_INFO, "ProcessInfo(): Encoded by: %s",ov_comment(&m_vf,-1)->vendor);
	CurrentSoundStream->SetBitRate(vi->bitrate_nominal);
	CurrentSoundStream->SetSampleRate(vi->rate);
	CurrentSoundStream->SetNumberOfChannels(vi->channels);
}

void COGGStreamReader::ReadComments()
{
	char **ptr=ov_comment(&m_vf,-1)->user_comments;
	while(*ptr)
	{
		Log(LOG_INFO, "%s",*ptr);
		if (strstr(*ptr, "TITLE="))
		{
			CurrentSoundStream->SetTitle((*ptr)+strlen("TITLE="));
		}
		else if (strstr(*ptr, "ARTIST="))
		{
			CurrentSoundStream->SetArtist((*ptr)+strlen("ARTIST="));
		}
		++ptr;
	}
}

void COGGStreamReader::Close()
{
	//CurrentSoundStream->Close();
	if (CPSPSoundStream::STREAM_STATE_OPEN == CurrentSoundStream->GetState())
	{
		//if (CPSPSoundStream::STREAM_TYPE_FILE == CurrentSoundStream->GetType())
		{
			m_lock->Lock(); /** Vorbis is not thread safe */
			ov_clear(&m_vf);
			m_lock->Unlock(); /** Vorbis is not thread safe */
			CurrentSoundStream->SetState(CPSPSoundStream::STREAM_STATE_CLOSED);
		}
	}	
}

size_t COGGStreamReader::Read(unsigned char *pBuffer, size_t SizeInBytes)
{
	size_t BytesRead = 0;
	long lRet = 0;
	int current_section = m_last_section;
	
	while ( (BytesRead < SizeInBytes) && (false == m_eof) )
	{
		m_lock->Lock(); /** Vorbis is not thread safe */
		lRet = ov_read(&m_vf, (char *)(pBuffer+BytesRead), SizeInBytes-BytesRead, &current_section);
	
		if ((current_section != m_last_section) && (lRet != 0))
		{
			Log(LOG_LOWLEVEL, "m_last_section changed from %d to %d. Processing Changes...", 
				m_last_section, current_section);
			ReadComments(); /** Get new stream information */
			ReadComments(); /** Get metadata from the stream */
			m_last_section = current_section;
			pPSPSound->SendEvent(MID_NEW_METADATA_AVAILABLE);
		}
		m_lock->Unlock();
		
		switch(lRet)
		{
			case 0:
				Log(LOG_INFO, "OGG Stream End.. Closing..");
				Close();
				m_eof = true;
				break;
			case OV_HOLE:
				//indicates there was an interruption in the data.
				//(one of: garbage between pages, loss of sync followed by recapture, or a corrupt page)
				Log(LOG_LOWLEVEL, "OGG Stream Warning: OV_HOLE (Garbage/loss of sync/corrupt page)");
				//Close();
				//m_eof = true;
				//return BytesRead;
				break;
			case OV_EBADLINK:
				//indicates that an invalid stream section was supplied to libvorbisidec, or 
				//the requested link is corrupt.
				Log(LOG_LOWLEVEL, "OGG Stream Warning: OV_EBADLINK (Invalid stream section/corrupted link)");
				//Close();
				//m_eof = true;
				break;
			default:
				BytesRead += lRet;
				break;
		}
		
	}
	
	//Log(LOG_VERYLOW, "Read. (End) bitstream=%d", bitstream);
	return BytesRead;
}

/** ---------------------------------------------------------------------------------- **/
void CPSPSoundDecoder_OGG::Initialize()
{
	Log(LOG_LOWLEVEL, "CPSPSoundDecoder_OGG Initialize"); 

	m_InputStreamReader = new COGGStreamReader();

	if (!m_InputStreamReader)
	{
		Log(LOG_ERROR, "CPSPSoundDecoder_OGG::Memory allocation error instantiating m_InputStream");
		ReportError("CPSPSoundDecoder_OGG::Decoder Initialization Error");
	}
	
	m_pInputBuffer = (unsigned char*)malloc(INPUT_BUFFER_SIZE);
	if (!m_pInputBuffer)
	{
		ReportError("Memory allocation error!\n");
		Log(LOG_ERROR, "Memory allocation error!\n");
		return;
	}
}
			
CPSPSoundDecoder_OGG::~CPSPSoundDecoder_OGG()
{
	Log(LOG_VERYLOW, "~CPSPSoundDecoder_OGG start");
	
	if (m_pInputBuffer)
	{
		free(m_pInputBuffer), m_pInputBuffer = NULL;
	}
	Log(LOG_VERYLOW, "~CPSPSoundDecoder_OGG end");
}

bool CPSPSoundDecoder_OGG::Decode()
{
	long lFramesDecoded = 0;
	Sample *pOutputSample = (Sample *)m_pInputBuffer;
	Sample SampleL = 0, SampleR = 0;
	
	PCMFrameInSamples PCMOutputFrame;/** The output buffer holds one BUFFER */
	
	long lRet = 0;
	
	int iNumberOfChannels = CurrentSoundStream->GetNumberOfChannels();
	
	lRet = m_InputStreamReader->Read(m_pInputBuffer, 4096);
	
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
	
	return m_InputStreamReader->IsEOF();
}
