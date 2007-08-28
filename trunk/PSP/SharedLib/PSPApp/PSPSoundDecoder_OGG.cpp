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
#include <sys/socket.h>
#include "PSPSoundDecoder_OGG.h"
#ifdef USE_TREMOR
	#include <ivorbisfile.h>
#else
	#include <vorbis/vorbisfile.h>
#endif
#include "bstdfile.h"
#include <malloc.h>
using namespace std;

size_t sg_lBytesReadFromStream = 0;

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
	return close(*(int*)pSocket);
}

long ogg_socket_tell_wrapper(void *datasource)
{
	return sg_lBytesReadFromStream;
}

COGGStreamReader::COGGStreamReader(CPSPStream *InputStream):CPSPStreamReader(InputStream)
{
	sg_lBytesReadFromStream = 0; /** Reset number of bytes read from stream */
	m_last_section = -1;
	m_lock = new CLock("OggReaderLock");
	m_eof = false;
	m_InputStream = InputStream;
	m_iMetaDataInterval = m_InputStream->GetMetaDataInterval();
	m_iRunningCountModMetadataInterval = 0;
	memset(bMetaData, 0, MAX_METADATA_SIZE);
 	memset(bPrevMetaData, 0, MAX_METADATA_SIZE);
	m_pfd = m_InputStream->GetFileDescriptor();
	m_fdSocket = m_InputStream->GetSocketDescriptor();
	
	int iRet  = -1;
	m_lock->Lock(); /** Vorbis is not thread safe */
	switch (m_InputStream->GetType())
	{
		case CPSPStream::STREAM_TYPE_FILE:
			iRet = ov_open(m_pfd, &m_vf, NULL /*char *initial*/, 0 /*long ibytes*/);
			break;
		
		case CPSPStream::STREAM_TYPE_URL:
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
			pPSPSound->SendEvent(MID_STREAM_TIME_UPDATED);
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
		m_InputStream->Close();
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
	m_InputStream->SetLength(ov_pcm_total(&m_vf,-1));
	Log(LOG_INFO, "ProcessInfo(): Encoded by: %s",ov_comment(&m_vf,-1)->vendor);
	m_InputStream->SetBitRate(vi->bitrate_nominal);
	m_InputStream->SetSampleRate(vi->rate);
	m_InputStream->SetNumberOfChannels(vi->channels);
	#ifdef USE_TREMOR
		m_InputStream->SetTotalTime(ov_time_total(&m_vf, -1) / 1000);
	#else
		m_InputStream->SetTotalTime((long)(ov_time_total(&m_vf, -1)));
	#endif
}

void COGGStreamReader::ReadComments()
{
	int iExpectedCount = 0;
	Log(LOG_INFO, "ReadComments()");
	char **ptr=ov_comment(&m_vf,-1)->user_comments;
	while(*ptr && (iExpectedCount < 2))
	{
		Log(LOG_INFO, "%s",*ptr);
		if (strstr(*ptr, "TITLE="))
		{
			m_InputStream->SetTitle((*ptr)+strlen("TITLE="));
			iExpectedCount++;
		}
		else if (strstr(*ptr, "ARTIST="))
		{
			m_InputStream->SetArtist((*ptr)+strlen("ARTIST="));
			iExpectedCount++;
		}
		++ptr;
	}
	Log(LOG_VERYLOW, "ReadComments() End.");
}

void COGGStreamReader::Close()
{
	//m_InputStream->Close();
	if (CPSPStream::STREAM_STATE_OPEN == m_InputStream->GetState())
	{
		//if (CPSPStream::STREAM_TYPE_FILE == m_InputStream->GetType())
		{
			m_lock->Lock(); /** Vorbis is not thread safe */
			ov_clear(&m_vf);
			m_lock->Unlock(); /** Vorbis is not thread safe */
			m_InputStream->SetState(CPSPStream::STREAM_STATE_CLOSED);
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
		#ifdef USE_TREMOR
			lRet = ov_read(&m_vf, (char *)(pBuffer+BytesRead), SizeInBytes-BytesRead, &current_section);
		#else
			lRet = ov_read(&m_vf, (char *)(pBuffer+BytesRead), SizeInBytes-BytesRead, 
							0, /** little endian */
							2, /** 16-bit samples */
							1, /** signed */
							&current_section);
		#endif
	
		if ((current_section != m_last_section) && (lRet != 0))
		{
			Log(LOG_LOWLEVEL, "m_last_section changed from %d to %d. Processing Changes...", 
				m_last_section, current_section);
			ProcessInfo(); /** Get new stream information */
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
				#ifdef USE_TREMOR
					long Time = ov_time_tell(&m_vf) / 1000;
				#else
					long Time = (long)(ov_time_tell(&m_vf));
				#endif
					if ((OV_EINVAL != Time) && (Time != m_InputStream->GetCurrentTime()))
					{
						m_InputStream->SetCurrentTime(Time);
						m_InputStream->SetBytePosition(ov_raw_tell(&m_vf));
						pPSPSound->SendEvent(MID_STREAM_TIME_UPDATED);
					}

				BytesRead += lRet;
				break;
		}
		
	}
	
	//Log(LOG_VERYLOW, "Read. (End) bitstream=%d", bitstream);
	return BytesRead;
}

void COGGStreamReader::Seek(int iPosition)
{
	m_lock->Lock(); /** Vorbis is not thread safe */
	ov_raw_seek(&m_vf, iPosition);
	m_lock->Unlock();
}

/** ---------------------------------------------------------------------------------- **/
void CPSPSoundDecoder_OGG::Initialize()
{
	Log(LOG_LOWLEVEL, "CPSPSoundDecoder_OGG Initialize"); 

	m_InputStreamReader = new COGGStreamReader(m_InputStream);

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
	
	static PCMFrameInSamples PCMOutputFrame;/** The output buffer holds one BUFFER */
	
	long lRet = 0;
	
	int iNumberOfChannels = m_InputStream->GetNumberOfChannels();
	
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
			
			m_Buffer->PushPCMFrame(*((::Frame*)&PCMOutputFrame));
			
		}

	}
	
	return m_InputStreamReader->IsEOF();
}

void CPSPSoundDecoder_OGG::Seek(int iPosition)
{
	m_InputStreamReader->Seek(iPosition);
}
