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
#include <PSPApp.h>
#include <PSPSound.h>
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>
#include <iniparser.h>
#include <Tools.h>
#include <Logging.h>
#include <pspwlan.h> 
#include <psphprm.h>
#include "ScreenHandler.h"
#include "DirList.h"
#include "PlayList.h"
#include "TextUI.h"
#include "GraphicsUI.h"
#include "SandbergUI.h" 

#define SHOUTCAST_DB_REQUEST_STRING				"http://www.shoutcast.com/sbin/xmllister.phtml?service=pspradio&no_compress=1"
#define SHOUTCAST_DB_COMPRESSED_REQUEST_STRING 	"http://www.shoutcast.com/sbin/xmllister.phtml?service=pspradio"
#define SHOUTCAST_DB_COMPRESSED_FILENAME		"SHOUTcast/db.xml.gz"
#define SHOUTCAST_DB_FILENAME					"SHOUTcast/db.xml"

bool UnCompress(char *strSourceFile, char *strDestFile);
int inf(FILE *source, FILE *dest);

bool CScreenHandler::DownloadSHOUTcastDB()
{
	bool success = false;
	CPSPSoundStream *WebConnection = new CPSPSoundStream();
	WebConnection->SetURI(SHOUTCAST_DB_COMPRESSED_REQUEST_STRING);
	WebConnection->Open();
	if (true == WebConnection->IsOpen())
	{
		Log(LOG_INFO, "DownloadSHOUTcastDB(): Connected - Downloading '%s'", SHOUTCAST_DB_COMPRESSED_FILENAME);
		bool bRet;
		size_t bytes;
		bRet = WebConnection->DownloadToFile(SHOUTCAST_DB_COMPRESSED_FILENAME, bytes);
		
		if (true == bRet)
		{
			WebConnection->Close();
			delete WebConnection, WebConnection = NULL;
			Log(LOG_INFO, "DownloadSHOUTcastDB(): DB Retrieved. (%dbytes)", bytes);
			m_UI->DisplayMessage("Uncompressing . . .");
			bRet = UnCompress(SHOUTCAST_DB_COMPRESSED_FILENAME, SHOUTCAST_DB_FILENAME);
			if (true == bRet)
			{
				m_UI->DisplayMessage("SHOUTcast DataBase Retrieved");
				Log(LOG_INFO, "SHOUTcast.com DB retrieved.");
				success = true;
			}
			else
			{
				Log(LOG_ERROR, "Error uncompressing '%s' to '%s'",SHOUTCAST_DB_COMPRESSED_FILENAME, SHOUTCAST_DB_FILENAME);
				m_UI->DisplayMessage("Error Uncompressing . . .");
			}
		}
	}
	else
	{
		Log(LOG_ERROR, "Error connecting to '%s'", SHOUTCAST_DB_COMPRESSED_REQUEST_STRING);
		m_UI->DisplayErrorMessage("Couldn't connect to SHOUTcast.com ...");
	}
	
	return success;
}


/** Raf: Thanks to echto for the uncompression routines! */
#include "zlib.h"

#define CHUNK 16384

bool UnCompress(char *strSourceFile, char *strDestFile)
{
	bool success = false;
	FILE *infile  = NULL;
	FILE *outfile = NULL;
		
	srand(sceKernelLibcTime(NULL)); // seeded with time

	infile  = fopen(strSourceFile,"r");
	
	if (infile)
	{
		outfile = fopen(strDestFile,"w");
		if (outfile)
		{
			/* do decompression */
			int ret = inf(infile, outfile);
			if (ret == Z_OK)
			{
				Log(LOG_LOWLEVEL, "UnCompress(): File '%s' decompressed successfully", strSourceFile);
				success = true;
			}
			else
			{
				/* report a zlib or i/o error */
				switch (ret) 
				{
					case Z_ERRNO:
						if (ferror(stdin))
							Log(LOG_ERROR, "UnCompress(): zlib error: error reading file");
						if (ferror(stdout))
							Log(LOG_ERROR, "UnCompress(): zlib error: error writing file");
						break;
					case Z_STREAM_ERROR:
						Log(LOG_ERROR, "UnCompress(): zlib error: invalid compression level");
						break;
					case Z_DATA_ERROR:
						Log(LOG_ERROR, "UnCompress(): zlib error: invalid or incomplete deflate data");
						break;
					case Z_MEM_ERROR:
						Log(LOG_ERROR, "UnCompress(): zlib error: out of memory");
						break;
					case Z_VERSION_ERROR:
						Log(LOG_ERROR, "UnCompress(): zlib error: zlib version mismatch!");
				}
				success = false;
			}
			fclose(outfile);
		}
		else
		{
			Log(LOG_ERROR, "UnCompress(): Unable to open file for writing '%s'", strDestFile);
		}
		fclose(infile);
	}
	else
	{
		Log(LOG_ERROR, "UnCompress(): Unable to open file for reading '%s'", strSourceFile);
	}

	return success;
}

int inf(FILE *source, FILE *dest)
{
    int ret;
    unsigned have;
    z_stream strm;
    Bytef in[CHUNK];
    Bytef out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
