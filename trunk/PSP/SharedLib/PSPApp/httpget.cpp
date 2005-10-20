/*
 *   httpget.c
 *
 *   Oliver Fromme  <oliver.fromme@heim3.tu-clausthal.de>
 *   Wed Apr  9 20:57:47 MET DST 1997
 
 *  Modified for PSPApp by Raf 2005.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pspnet.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <ctype.h>
#include <PSPApp.h>
#include <PSPSound.h>
#include <PSPSoundDecoder.h>
#include <Logging.h>

#define ProgName    pPSPApp->GetProgramName()
#define ProgVersion pPSPApp->GetProgramVersion()
#define ReportError pPSPApp->ReportError
#define SendEvent pPSPSound->SendEvent

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

int ConnectWithTimeout(SOCKET sock, struct sockaddr *addr, int size, size_t timeout/* in seconds */);

void writestring (int fd, char *string)
{
	int result, bytes = strlen(string);

	while (bytes) 
	{
		if ((result = send(fd, string, bytes, 0)) < 0 && sceNetInetGetErrno() != EINTR) 
		{
			ReportError("writestring(): write Error");
		}
		else if (result == 0) {
			ReportError ( "writestring(): write: %s\n",
				"socket closed unexpectedly");
		}
		string += result;
		bytes -= result;
		
		if (pPSPSound->GetPlayState() == CPSPSound::STOP || pPSPApp->IsExiting() == TRUE)
			break;
	}
}

void readstring (char *string, int maxlen, int sock)
{

	int pos = 0;
	int bytesread = 0;
	string[0] = 0;

	for(;;)
	{
		bytesread = recv(sock, string+pos, 1, 0);
		if(bytesread  == 1) {
			pos++;
			if(string[pos-1] == '\n') {
				string[pos] = 0;
				break;
			}
		}
		else if (bytesread == 0)
		{
			ReportError ( "Connection closed by peer!\n");
			break;
		}
		if (pPSPSound->GetPlayState() == CPSPSound::STOP || pPSPApp->IsExiting() == TRUE)
			break;
		//else if(errno != EINTR) 
		//{
		//	ReportError ( "Error reading from socket or unexpected EOF.\n");
		//	break;
		//}
	}
}

void encode64 (char *source,char *destination)
{
  static char *Base64Digits =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  int n = 0;
  int ssiz=strlen(source);
  int i;

  for (i = 0 ; i < ssiz ; i += 3) {
    unsigned int buf;
    buf = ((unsigned char *)source)[i] << 16;
    if (i+1 < ssiz)
      buf |= ((unsigned char *)source)[i+1] << 8;
    if (i+2 < ssiz)
      buf |= ((unsigned char *)source)[i+2];

    destination[n++] = Base64Digits[(buf >> 18) % 64];
    destination[n++] = Base64Digits[(buf >> 12) % 64];
    if (i+1 < ssiz)
      destination[n++] = Base64Digits[(buf >> 6) % 64];
    else
      destination[n++] = '=';
    if (i+2 < ssiz)
      destination[n++] = Base64Digits[buf % 64];
    else
      destination[n++] = '=';
  }
  destination[n++] = 0;
}

/* VERY  simple auth-from-URL grabber */
int getauthfromURL(char *url,char *auth)
{
  char *pos;

  *auth = 0;

  if (!(strncasecmp(url, "http://", 7)))
    url += 7;

  if (!(strncasecmp(url, "ftp://", 6)))
    url += 6;

  if( (pos = strchr(url,'@')) ) {
    int i;
    for(i=0;i<pos-url;i++) {
      if( url[i] == '/' )
         return 0;
    }
    strncpy(auth,url,pos-url);
    auth[pos-url] = 0;
    strcpy(url,pos+1);
    return 1;
  }
  return 0;
}

static char *defaultportstr = "80";

char *url2hostport (char *url, char **hname, unsigned long *hip, unsigned char **port)
{
	char *h, *p;
	char *hostptr;
	char *r_hostptr;
	char *pathptr;
	char *portptr;
	char *p0;
	size_t stringlength;

	p = url;
	if (strncasecmp(p, "http://", 7) == 0)
		p += 7;

        if (strncasecmp(p, "ftp://", 6) == 0)
               p += 6;

	hostptr = p;
	while (*p && *p != '/')
		p++;
	pathptr = p;

	r_hostptr = --p;
	while (*p && hostptr < p && *p != ':' && *p != ']')
		p--;

	if (!*p || p < hostptr || *p != ':') {
		portptr = NULL;
	}
	else{
		portptr = p + 1;
		r_hostptr = p - 1;
	}
	if (*hostptr == '[' && *r_hostptr == ']') {
		hostptr++;
		r_hostptr--;
	}

	stringlength = r_hostptr - hostptr + 1;
	h = (char*)malloc(stringlength + 1); /* removed the strndup for better portability */
	if (h == NULL) {
		*hname = NULL;
		*port = NULL;
		return NULL;
	}
	strncpy(h, hostptr, stringlength);
	*(h+stringlength) = '\0';
	*hname = h;

	if (portptr) {
		stringlength = (pathptr - portptr);
		if(!stringlength) portptr = NULL;
	}
	if (portptr == NULL) {
		portptr = defaultportstr;
		stringlength = strlen(defaultportstr);
	}
	p0 = (char*)malloc(stringlength + 1);
	if (p0 == NULL) {
		free(h);
		*hname = NULL;
		*port = NULL;
		return NULL;
	}
	strncpy(p0, portptr, stringlength);
	*(p0 + stringlength) = '\0';

	for (p = p0; *p && isdigit((unsigned char) *p); p++) ;

	*p = '\0';
	*port = (unsigned char *) p0;

	return pathptr;
}

char *proxyurl = NULL;
unsigned long proxyip = 0;
unsigned char *proxyport;

#define ACCEPT_HEAD "Accept: audio/mpeg, audio/x-mpegurl, */*\r\n"

char *httpauth = NULL;
char httpauth1[256];

int http_open (char *url, size_t &iMetadataInterval, CPSPSoundStream::content_types &ContentType)
{
	char *purl = NULL, 
		 *host = NULL, 
		 *request = NULL,
		 *sptr = NULL;
	int linelength = 0;
	unsigned long myip = 0l;
	unsigned char *myport = NULL;
	int sock = 0;
	int relocate = 0, numrelocs = 0;
#ifdef INET6
	struct addrinfo hints, *res, *res0;
	int error;
#else
	//struct hostent *hp;
	struct sockaddr_in sin;
#endif

	Log(LOG_LOWLEVEL, "http_open(%s) called.", url);

	host = NULL;
	proxyport = NULL;
	myport = NULL;
	if (!proxyip) {
		if (!proxyurl)
			if (!(proxyurl = getenv("MP3_HTTP_PROXY")))
				if (!(proxyurl = getenv("http_proxy")))
					proxyurl = getenv("HTTP_PROXY");
		if (proxyurl && proxyurl[0] && strcmp(proxyurl, "none")) {
			if (!(url2hostport(proxyurl, &host, &proxyip, &proxyport))) {
				ReportError ( "Unknown proxy host \"%s\".\n",
					host ? host : "");
				return -1;
			}
#if 0
			if (host)
				free (host);
#endif
		}
		else
			proxyip = INADDR_NONE;
	}


	if (proxyip == INADDR_NONE)
	{
		if (strncasecmp(url, "ftp://", 6) == 0)
		{
			ReportError ("Downloading from ftp servers without PROXY not allowed\n");
			return -1;
		}
	}
	
	if ((linelength = strlen(url)+256) < 4096)
		linelength = 4096;
	if (!(request = (char*)malloc(linelength)) || !(purl = (char*)malloc(1024))) {
		ReportError ( "malloc() failed, out of memory.\n");
		return -1;
	}
	memset(request, 0, linelength);
	memset(purl, 0, 1024);
	
       /*
        * 2000-10-21:
        * We would like spaces to be automatically converted to %20's when
        * fetching via HTTP.
        * -- Martin Sj�ren <md9ms@mdstud.chalmers.se>
        */
       if ((sptr = strchr(url, ' ')) == NULL) {
               strncpy (purl, url, 1023);
               purl[1023] = '\0';
       }
       else {
               int purllength = 0;
               char *urlptr = url;
               purl[0] = '\0';
               do {
                       purllength += sptr-urlptr + 3;
                       if (purllength >= 1023)
                               break;
                       strncat (purl, urlptr, sptr-urlptr);
                       //purl[sptr-url] = '\0';
                       strcat (purl, "%20");
                       urlptr = sptr + 1;
               }
               while ((sptr = strchr (urlptr, ' ')) != NULL);
               strcat (purl, urlptr);
       }


        getauthfromURL(purl,httpauth1);

	do {
		strcpy (request, "GET ");
		if (proxyip != INADDR_NONE) 
		{
			if (strncasecmp(url, "http://", 7) != 0 && strncasecmp(url,"ftp://", 6) != 0)
			strcat (request, "http://");
			strcat (request, purl);
			myport = proxyport;
			myip = proxyip;
		}
		else {
			if (host) {
				free(host);
				host=NULL;
			}
			if (proxyport) {
				free(proxyport);
				proxyport=NULL;
			}
			//ReportError("purl+7='%s'\n", purl+7);
			if(strchr(purl+7,'/')==NULL)
			{
				strcat(purl, "/");
			} 
			if (!(sptr = url2hostport(purl, &host, &myip, &myport))) 
			{
				ReportError ( "Unknown host \"%s\".\n",
					host ? host : "");
				return -1;
			}
			
			Log(LOG_LOWLEVEL, "url2hostport returns: host='%s' ip='0x%x' port='%s' sptr='%s'", host, myip, myport, sptr);
			strcat (request, sptr);
		}
		
		/** RC: Request metadata */
		sprintf (request + strlen(request),
			" %s\r\n%s\r\nUser-Agent: %s/%s\r\n",
			"HTTP/1.0",
			"Icy-MetaData: 1",
			ProgName, ProgVersion);

		
		if (host) 
		{
			sprintf(request + strlen(request),
				"Host: %s:%s\r\n", host, myport);
		}
		
		strcat (request, ACCEPT_HEAD);

#ifdef INET6
		memset(&hints, 0, sizeof(hints));
		hints.ai_socktype = SOCK_STREAM;
		error = getaddrinfo(host, (char *)myport, &hints, &res0);
		if (error) {
			ReportError ( "getaddrinfo: %s\n", gai_strerror(error));
			return -1;
		}

		sock = -1;
		for (res = res0; res; res = res->ai_next) {
			if ((sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
				continue;
			}
			if (connect(sock, res->ai_addr, res->ai_addrlen)) {
				close(sock);
				sock = -1;
				continue;
			}
			break;
		}

		freeaddrinfo(res0);
#else
		sock = -1;
		in_addr addr;
		int rc = 0;
		memset(&addr, 0, sizeof(in_addr));

		/* RC Let's try aton first in case the address is in dotted numerical form */
		Log(LOG_LOWLEVEL, "Calling aton.. (host='%s')",host);
		memset(&addr, 0, sizeof(in_addr));
		rc = inet_aton(host, &addr);
		if (rc == 0)
		{
			/** That didn't work!, it must be a hostname, let's try the resolver... */
			Log(LOG_LOWLEVEL, "http_connect(): Calling sceNetResolverStartNtoA() with resolverid = %d",
				 pPSPApp->GetResolverId());
			rc = sceNetResolverStartNtoA(pPSPApp->GetResolverId(), host, &addr, 2, 3);
			if (rc < 0)
			{
				ReportError("Could not resolve host!\n");
				goto fail;
			}
			
		}
		Log(LOG_LOWLEVEL, "aton/ntoa succeeded, returned addr='0x%x'", addr);

		
		Log(LOG_LOWLEVEL, "http_connect(): Opening socket...");
		//ReportError ("Resolved host's IP: '%s'\n", inet_ntoa(addr));
		sock = socket(AF_INET, SOCK_STREAM, 0);
		Log(LOG_LOWLEVEL, "http_connect(): Aquired socket fd=%d...", sock);
		if (sock < 0)
			goto fail;
		
		struct timeval  timeo;
		timeo.tv_sec  = 3; //sec
		timeo.tv_usec = 0; //usec
		Log(LOG_LOWLEVEL, "http_connect(): Calling Setsockopt()");
		if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo)) < 0) 
		{
			ReportError("setsockopt SO_RCVTIMEO Failed");
		}
		if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo)) < 0) 
		{
			ReportError("setsockopt SO_SNDTIMEO Failed");
		}
		
		
		memset(&sin, 0, sizeof(struct sockaddr_in));
		sin.sin_family = AF_INET;
		sin.sin_len = sizeof(struct sockaddr_in);
		memcpy(&sin.sin_addr, &addr, sizeof(in_addr));
		
		Log(LOG_LOWLEVEL, "Using port '%s'\n", myport);
        sin.sin_port = htons(atoi( (char *) myport));
        
		/** Bail out if app exiting */		
		if (pPSPSound->GetPlayState() == CPSPSound::STOP || pPSPApp->IsExiting() == TRUE)
		{
			sock = -1;
			return sock;
		}
		
        Log(LOG_LOWLEVEL, "http_connect(): Calling Connect... with port='%s', sock=%d, addr='0x%x'", myport, sock, addr);
        int iCon = ConnectWithTimeout(sock, (struct sockaddr *)&sin, sizeof(struct sockaddr_in), 5/*5 sec timeout */ );
		if (iCon != 0) 
		{
			ReportError("Error Connecting - Timeout.");
			Log(LOG_ERROR, "Connection timeout. iCon=0x%x", iCon);
			sceNetInetClose(sock);
			sock = -1;
		}
        Log(LOG_LOWLEVEL, "http_connect(): Back from Connect...");

fail:
#endif

		if (sock < 0) 
		{
			return -1;
		}

		if (strlen(httpauth1) || httpauth) 
		{
			char buf[1023];
			strcat (request,"Authorization: Basic ");
			if(strlen(httpauth1))
				encode64(httpauth1,buf);
			else
				encode64(httpauth,buf);
			strcat (request,buf);
			strcat (request,"\r\n");
		}
		strcat (request, "\r\n");
		
		/** Bail out if app exiting */
		if (pPSPSound->GetPlayState() == CPSPSound::STOP || pPSPApp->IsExiting() == TRUE)
		{
			sock = -1;
			return sock;
		}
		Log(LOG_LOWLEVEL, "http_connect(): Sending '%s'", request);
		writestring (sock, request);
		
		//if (!(myfile = fdopen(sock, "rb"))) {
		//	perror ("fdopen");
		//	exit (1);
		//};
		relocate = FALSE;
		purl[0] = '\0';
		request[0] = 0;
		/** Bail out if app exiting */
		if (pPSPSound->GetPlayState() == CPSPSound::STOP || pPSPApp->IsExiting() == TRUE)
		{
			sock = -1;
			return sock;
		}
		readstring (request, linelength-1, sock);
		if (strlen(request) == 0)
		{
			ReportError("Error, no response.\n");
			return -1;
		}
		
		Log(LOG_LOWLEVEL, "http_connect(): Response: %s", request);
		sptr = strchr(request, ' ');
		if (sptr != NULL) 
		{
			switch (sptr[1]) {
				case '3':
					relocate = TRUE;
				case '2': /** OK */
					break;
				default:
					Log (LOG_ERROR, "HTTP request failed: %s",
						sptr+1); /* '\n' is included */
					ReportError("HTTP Request Failed.");
					return -1;
			}
		}
		
		do 
		{
			/** Bail out if app exiting */
			if (pPSPSound->GetPlayState() == CPSPSound::STOP || pPSPApp->IsExiting() == TRUE)
			{
				sock = -1;
				return sock;
			}
			memset(request, 0, linelength);
			readstring (request, linelength-1, sock);
			
			Log(LOG_LOWLEVEL, "http_connect(): Response: %s", request);
			if (strncmp(request, "Location:", 9) == 0)
			{
				strncpy (purl, request+10, 1023);
			}
			else if (strncmp(request, "icy-metaint:", 12) == 0)
			{
				sscanf(request, "icy-metaint: %d", &iMetadataInterval);
				Log(LOG_INFO, "http_connect(): Metadata Interval received: %d", iMetadataInterval);
			}
			else if (0 == strncmp(request, "content-type:", strlen("content-type:")) ||
					 0 == strncmp(request, "Content-Type:", strlen("content-type:")))
			{
				char *content = request+strlen("content-type:");
				if (content[0] == ' ')
					content++;
				Log(LOG_VERYLOW, "Parsing content:%s",content);
				if (0 == strncmp(content, "audio/mpeg", strlen("audio/mpeg")))
				{
					ContentType = CPSPSoundStream::STREAM_CONTENT_AUDIO_MPEG;
					Log(LOG_INFO, "Content Type set to audio/mpeg");
				}
				else if (0 == strncmp(content, "application/ogg", strlen("application/ogg")))
				{
					ContentType = CPSPSoundStream::STREAM_CONTENT_AUDIO_OGG;
					Log(LOG_INFO, "Content Type set to application/ogg");
				}
				else if (0 == strncmp(content, "audio/aac", strlen("audio/aac")))
				{
					ContentType = CPSPSoundStream::STREAM_CONTENT_AUDIO_AAC;
					Log(LOG_INFO, "Content Type set to audio/aac");
				}
			}
		} while (request[0] != '\r' && request[0] != '\n');
	} while (relocate && purl[0] && numrelocs++ < 5);
	if (relocate) {
		ReportError ( "Too many HTTP relocations.\n");
		return -1;
	}
	free (purl);
	free (request);
	free(host);
	free(proxyport);
	free(myport);

	return sock;
}

/* EOF */

/** Based on Code from VNC for PSP*/
int ConnectWithTimeout(SOCKET sock, struct sockaddr *addr, int size, size_t timeout/* in s */) 
{
	u32 err = 0;
	int one = 1, zero = 0;
	sceNetInetSetsockopt(sock, SOL_SOCKET, SO_NONBLOCK, (char *)&one, sizeof(one));
	
	err = sceNetInetConnect(sock, addr, sizeof(struct sockaddr));
	if (err == 0)
	{
		sceNetInetSetsockopt(sock, SOL_SOCKET, SO_NONBLOCK, (char *)&zero, sizeof(zero));
		SendEvent(MID_TCP_CONNECTING_SUCCESS);
		return 0;
	}
	
	if (err == 0xFFFFFFFF && sceNetInetGetErrno() == 0x77)
	{
		size_t ticks;
		for (ticks = 0; ticks < timeout; ticks++) 
		{
			
			err = sceNetInetConnect(sock, addr, sizeof(struct sockaddr));
			if (err == 0 || (err == 0xFFFFFFFF && sceNetInetGetErrno() == 0x7F)) 
			{
				sceNetInetSetsockopt(sock, SOL_SOCKET, SO_NONBLOCK, (char *)&zero, sizeof(zero));
				SendEvent(MID_TCP_CONNECTING_SUCCESS);
				return 0;
			}
			sceKernelDelayThread(1000000); /* 1 s */
			SendEvent(MID_TCP_CONNECTING_PROGRESS);
		}
	}
	
	//Log(LOG_LOWLEVEL, "Could not connect (Timeout?) geterrno = 0x%x", sceNetInetGetErrno());
	
	sceNetInetSetsockopt(sock, SOL_SOCKET, SO_NONBLOCK, (char *)&zero, sizeof(zero));
	SendEvent(MID_TCP_CONNECTING_FAILED);
	return err;
}
