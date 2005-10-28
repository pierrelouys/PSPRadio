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
#include <mad.h>
#include <malloc.h>
#include <errno.h>
#include <sys/socket.h>
#include "PSPSound.h"
#include "PSPStream.h"

#define METADATA_STREAMURL_TAG "StreamUrl='"
#define METADATA_STREAMTITLE_TAG "StreamTitle='"

/** class CPSPStream */
CPSPStream::CPSPStream()
{
	m_Type = STREAM_TYPE_NONE;
	m_State = STREAM_STATE_CLOSED;
	m_pfd = NULL;
	m_fdSocket = -1;
	m_MetaData = new MetaData;
	ClearMetadata();

	/** reader part */
	m_BstdFile = NULL;
	m_eof = false;
	m_iMetaDataInterval = 0;
	m_iRunningCountModMetadataInterval = 0;
	memset(bMetaData, 0, MAX_METADATA_SIZE);
	memset(bPrevMetaData, 0, MAX_METADATA_SIZE);
	
}

CPSPStream::~CPSPStream()
{
	if (IsOpen())
		Close();
	
	if (m_MetaData)
	{
		delete(m_MetaData);
	}
}

void CPSPStream::ClearMetadata()
{
	memset(m_MetaData, 0, sizeof(m_MetaData));
}

/** Accessors */
void CPSPStream::SetURI(char *strFile)
{
	if (strFile)
	{
		if (strlen(strFile) > 4)
		{
			/** New URI, clear Metadata */
			ClearMetadata();
			strncpy(m_MetaData->strURI, strFile, MAXPATHLEN);
			if (memcmp(m_MetaData->strURI, "http://", strlen("http://")) == 0)
			{
				Log(LOG_LOWLEVEL, "CPSPStream::SetFile(%s) <URL> called", m_MetaData->strURI);
				m_Type = STREAM_TYPE_URL;
			}
			else // It's a file!
			{
				Log(LOG_LOWLEVEL, "CPSPStream::SetFile(%s) <FILE> called", m_MetaData->strURI);
				m_Type = STREAM_TYPE_FILE;
			}
		}
		else
		{
			Log(LOG_ERROR, "CPSPStream::SetFile(%s) BAD.", strFile);
			ReportError("CPSPStream::OpenFile-Invalid filename '%s'", strFile);
		}
		
	}
}

bool CPSPStream::DownloadToFile(char *strFilename, size_t &bytesDownloaded)
{
	bool success = false;
	
	if (STREAM_TYPE_URL == GetType())
	{
		if (true == IsOpen())
		{
			FILE *fOut = fopen(strFilename, "w");
			if (fOut)
			{
				int iRet = 0;
				bytesDownloaded = 0;
				char *buffer = (char*)malloc(8192);
				memset(buffer, 0, 8192);
				
				for(;;)
				{
					iRet = recv(m_fdSocket, buffer, 8192, 0);
					if (0 == iRet)
					{
						fclose(fOut), fOut = NULL;
						success = true;
						break;
					}
					if (iRet > 0)
					{
						bytesDownloaded+= iRet;
						fwrite(buffer, iRet, 1, fOut);
					}
				}
				free(buffer), buffer = NULL;
			}
			else
			{
				Log(LOG_ERROR, "DownloadToFile(): Error- Couldn't open file for write.");
				success = false;
			}
		}
		else
		{
			Log(LOG_ERROR, "DownloadToFile(): Error- Couldn't connect");
			success = false;
		}
	}
	else
	{
		Log(LOG_ERROR, "DownloadToFile(): This is not a URL");
		success = false;
	}
	
	return success;
}

void CPSPStream::SetSampleRate(int SampleRate)
{
	if (m_MetaData->iSampleRate != SampleRate)
	{
		m_MetaData->iSampleRate = SampleRate; 
		pPSPSound->SampleRateChange();
	}
}

void CPSPStream::Close()
{
	if (STREAM_STATE_OPEN == m_State)
	{
		switch(m_Type)
		{
			case STREAM_TYPE_FILE:
				if (m_pfd)
				{
					if (m_BstdFile)
					{
						BstdFileDestroy(m_BstdFile);
						m_BstdFile = NULL;
					}
					fclose(m_pfd);
				}
				m_pfd = NULL;
				m_State = STREAM_STATE_CLOSED;
				break;
			case STREAM_TYPE_URL:
				if (m_fdSocket >= 0)
				{
					sceNetInetClose(m_fdSocket);
				}
				m_fdSocket = -1;
				m_State = STREAM_STATE_CLOSED;
				m_eof = true;
				break;
			case STREAM_TYPE_NONE:
				Log(LOG_ERROR, "SoundStream::Close(): Invalid State.");
				break;
		}
	}
	m_iRunningCountModMetadataInterval = 0;
	m_iMetaDataInterval = 0;
	memset(bMetaData, 0, MAX_METADATA_SIZE);
	memset(bPrevMetaData, 0, MAX_METADATA_SIZE);
}

int CPSPStream::Open()
{
	SetContentType(MetaData::CONTENT_NOT_DEFINED);
	if (STREAM_STATE_CLOSED == m_State)
	{
		switch(m_Type)
		{
			case STREAM_TYPE_URL:
				//ReportError ("Opening URL '%s'\n", filename);
				m_iMetaDataInterval = 0;
				m_iRunningCountModMetadataInterval = 0;
				m_fdSocket = http_open(m_MetaData->strURI);
				Log(LOG_LOWLEVEL, "Back from http_open(): socket=%d", m_fdSocket);
				if (m_fdSocket < 0)
				{
					//Don't report again, because http_open will report.
					//ReportError("CPSPStream::OpenFile-Error opening URL.\n");
					m_State = STREAM_STATE_CLOSED;
				}
				else
				{
					//ReportError("CPSPStream::OpenFile-URL Opened. (handle=%d)\n", m_fdSocket);
					//Log("Opened. MetaData Interval = %d\n", m_iMetaDataInterval);
					m_State = STREAM_STATE_OPEN;
				}
				break;
			
			case STREAM_TYPE_FILE:
				m_pfd = fopen(m_MetaData->strURI, "rb");
				if(m_pfd)
				{
					char *ext = strrchr(m_MetaData->strURI, '.') + 1;
					if (strlen(ext) >= 3)
					{
						if (0 == strncasecmp(ext, "mp", 2))
						{
							SetContentType(MetaData::CONTENT_AUDIO_MPEG);
						}
						else if (0 == strncasecmp(ext, "ogg", 3))
						{
							SetContentType(MetaData::CONTENT_AUDIO_OGG);
						}
					}
					m_BstdFile=NewBstdFile(m_pfd);
					if(m_BstdFile)
					{
						m_State = STREAM_STATE_OPEN;
					}
					else
					{
						ReportError("CPSPStream::OpenFile-Can't create a new bstdfile_t (%s).",
								strerror(errno));
						m_State = STREAM_STATE_CLOSED;
						Close();
					}
					
				}
				else
				{
					ReportError("Unable to open %s", m_MetaData->strURI);
				}
				break;
			case STREAM_TYPE_NONE:
				ReportError("Calling OpenFile, but the set filename is invalid '%s'", m_MetaData->strURI);
				break;
		}
	}
	else
	{
		ReportError("Calling OpenFile, but there is a file open already");
	}
	
	return m_State!=STREAM_STATE_CLOSED?0:-1;
}

bool CPSPStream::IsOpen()
{
	return (m_State==STREAM_STATE_CLOSED)?false:true;
}

/** ----------------------------------------------------------------------------------- */
size_t CPSPStream::Read(unsigned char *pBuffer, size_t SizeInBytes)
{
	size_t size = 0;
	char bMetaDataSize = 0;
	int iReadRet = -1;
	
	if (STREAM_STATE_OPEN == GetState())
	{
		switch(GetType())
		{
			case STREAM_TYPE_FILE:
				size = BstdRead(pBuffer, 1, SizeInBytes, m_BstdFile);
				break;
			case STREAM_TYPE_URL:
				if (m_iMetaDataInterval != 0)
				{
					m_iRunningCountModMetadataInterval = (m_iRunningCountModMetadataInterval % m_iMetaDataInterval);

					if (SizeInBytes + m_iRunningCountModMetadataInterval > m_iMetaDataInterval)
					{
						size = SocketRead((char*)pBuffer, m_iMetaDataInterval - m_iRunningCountModMetadataInterval);
						if (size != (m_iMetaDataInterval - m_iRunningCountModMetadataInterval))
						{
							Close();
							m_eof = true;
						}
						iReadRet = SocketRead(&bMetaDataSize, 1);
						if (iReadRet > 0)
						{
							iReadRet = SocketRead(bMetaData, bMetaDataSize * 16);
						}
						if (iReadRet != bMetaDataSize * 16)
						{
							Close();
							m_eof = true;
						}
						else
						{
							/** If new data is received */
							if (memcmp(bPrevMetaData, bMetaData, MAX_METADATA_SIZE) != 0)
							{
								Log(LOG_INFO, "MetaData='%s'", bMetaData);
								memcpy(bPrevMetaData, bMetaData, MAX_METADATA_SIZE);
								
								/** GetMetadataValue() modifies the metadata, so call it
								with the last tag first */
								char *tempMData = (char*)malloc(MAX_METADATA_SIZE);
								if (tempMData)
								{
									char *strURL= "", *strTitle= "";
									memcpy(tempMData, bMetaData, MAX_METADATA_SIZE);
									strURL   = GetMetadataValue(tempMData, METADATA_STREAMURL_TAG);
									strTitle = GetMetadataValue(tempMData, METADATA_STREAMTITLE_TAG);
									
									SetURL(strURL);
									SetTitle(strTitle);
									
									free(tempMData), tempMData = NULL;
									
									pPSPSound->SendEvent(MID_NEW_METADATA_AVAILABLE);
								}
							}
						}
					}
					else
					{
						size = SocketRead((char*)pBuffer, SizeInBytes);
						if (size != SizeInBytes)
						{
							Close();
							m_eof = true;
						}
					}
					if (size > 0)
					{
						m_iRunningCountModMetadataInterval+=size;
					}
				}
				else
				{
					size = SocketRead((char*)pBuffer, SizeInBytes);
					if (size != SizeInBytes)
					{
						Close();
						m_eof = true;
					}
				}
				break;
			case CPSPStream::STREAM_TYPE_NONE:
				Log(LOG_ERROR, "Read() Called, but no stream set up.");
				break;
		}
	}
	else
	{
		Log(LOG_ERROR, "Read() Called but the stream is not open!");
	}
	
	return size;
}

bool CPSPStream::IsEOF()
{
	int iseof = 0;
	
	if (STREAM_STATE_OPEN == GetState())
	{
		switch(GetType())
		{
			case STREAM_TYPE_FILE:
				iseof = BstdFileEofP(m_BstdFile);
				break;
			case STREAM_TYPE_URL:
				iseof = m_eof;
				break;
			case STREAM_TYPE_NONE:
				Log(LOG_ERROR, "IsEOF() Called but stream not setup");
				iseof = 1; /** Make them stop! */
				break;
		}
	}
	else
	{
		Log(LOG_ERROR, "IsEOF() Called but stream not open");
		iseof = 1; /** Make them stop! */
	}
	
	return iseof?true:false;
}

int CPSPStream::SocketRead(char *pBuffer, size_t LengthInBytes)
{
	size_t bytesread = 0, bytestoread = 0;
	size_t size = 0;

	if (true == IsOpen())
	{
		for(;;) 
		{
			if (pPSPSound->GetPlayState() == CPSPSound::STOP || pPSPApp->IsExiting() == true)
				break;
			
			bytestoread = LengthInBytes-size;
			bytesread = recv(m_fdSocket, pBuffer+size, bytestoread, 0);
			if (bytesread > 0)
				size += bytesread;
			if(bytesread == bytestoread) 
			{
				//done
				break;
			}
			else if (bytesread == 0)
			{
				ReportError("SocketRead(): Connection reset by peer!");
				Close();
				//m_eof = true;
				break;
			}
			else if(bytesread < 0)
			{
				errno = sceNetInetGetErrno();
				if (11 == errno)
				{
					/** Timeout! */
					continue;
				}
				else
				{
					ReportError ( "Error reading from socket or unexpected EOF.");
					//m_eof = true;
					Close();
					break;
				}
			}	
	
		}
	}
	return size;
}

/** Raw metadata looks like this:
 *  "StreamTitle='title of the song';StreamUrl='url address';"
 */
char *CPSPStream::GetMetadataValue(char *strMetadata, char *strTag)
{
	char *ret = "Parse Error";
	
	if (strMetadata && 
		strTag && 
		(strlen(strMetadata) > strlen(strTag)) && 
		strstr(strMetadata, strTag))
	{
		ret = strstr(strMetadata, strTag) + strlen(strTag);
		if (strchr(ret, ';'))
		{
			*(strchr(ret, ';') - 1) = 0;
		}
	}
	
	return ret;
}
