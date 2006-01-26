/*
   This program will parse out ID3v1 and v2 tags from any file if they exist.
   The v1 tag information will be put in the "char * info[5]" variable, since
   v1 tags only have 5 bits of information.  The v2 frames are put into the 
   "struct id2Frame frames[NUMFRAMES]", with each id2Frame struct containing
   the name of the frame (in it's 4 character code, which can be translated
   using getFrameName) and the frame data.  If you want to do anything with an
   individual frame, the best place is probably the printFrames function, as
   that will cycle through every frame, and you can check to see if it's the
   one you want.  the v1 tags can be dealt with anywhere, as they are a simple
   array.
   info[0] = song title
   info[1] = artist
   info[2] = album
   info[3] = year
   info[4] = comment
   gen = genre number, which can be translated using getGenre.
   		getGenre should be passed the genre number, and a character
		buffer which will be copied the result.

	Credits: Derek Davis (dmd2@email.byu.edu)
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <PSPStream.h>
#include <Logging.h>

#define FILECHUNK 3000 //How big of a chunk to read from beginning and end of file
#define FRAMEDATALENGTH 2000 //How big to allow v2 data to be
#define NUMFRAMES 20 //Max number of v2 frames

#define INDEX(x) x/8
#define SHIFT(x) x%8
#define GETBIT(v,x) ((v[INDEX(x)] << SHIFT(x)) & 0x80)>>7 //Macros to check if a bit is
#define SETBIT(v,x) (v[INDEX(x)] |= (0x80 >> SHIFT(x))) //set, and set a bit respectively
												//They are passed an array of characters,
struct id2Frame//Store v2 frames 				//and count starts from Left, not right.
{												//(dumb, I know)
	char fname[5];
	char data[FRAMEDATALENGTH];
} frames[NUMFRAMES];

unsigned char cFileBegin[FILECHUNK];
unsigned char cFileEnd[FILECHUNK];
int len, gen, numframes;
char *info[5];

void getGenre(int val, char * out);
char * getFrameName(char fname[]);

static MetaData *gMetaData = NULL;

void readFileEnd(char *fname)//Get chunk from end of file
{
    FILE* inf;
	long temp;
	temp=-128;
    inf=fopen(fname, "r");
    fseek(inf, temp, SEEK_END);
    len=128;
    fread(cFileEnd, sizeof(unsigned char), len, inf);
	fclose(inf);
}

void readFileBegin(char *fname)//Get chunk from beginning of file
{
    FILE* inf;
    inf=fopen(fname, "r");
    fseek(inf, 0L, SEEK_SET);
    len=FILECHUNK;
    fread(cFileBegin, 1, len, inf);
	fclose(inf);
}

int synchsafeToNormal(char tagSize[4]) //Convert size from synchsafe, which means
{								//that each 8th bit is zero, so everything needs
	int synchsafe, sizeloc, size, power, x; // to be shifted

	size=sizeloc=0;
	for(synchsafe=31;synchsafe>=0;synchsafe--)
	{
		if(GETBIT(tagSize, synchsafe))
		{
			power=1;
			for(x=0;x<sizeloc;x++) power*=2;
			size+=power;
		}
		if(synchsafe%8) sizeloc++;
	}
	return size;
}

void printData() //Print the data parsed from a v1 tag
{
	/*
	char * genre;

	genre=(char *)calloc(50, sizeof(char));
	getGenre(gen, genre);
	Log(LOG_INFO, 
		"\n\nID3v1:\nTitle - %s\nArtist - %s\nAlbum - %s\nYear - %s\nComment - %s\nGenre - %s\n\n\n", 
		info[0], info[1], info[2],
		info[3], info[4], genre);
		*/
	if (gMetaData)
	{
		getGenre(gen, gMetaData->strGenre);
		strncpy(gMetaData->strTitle, info[0], 300); gMetaData->strTitle[299] = 0;
		strncpy(gMetaData->strArtist, info[1], 300); gMetaData->strArtist[299] = 0;
		//strncpy(gMetaData->strGenre, genre, 128); gMetaData->strGenre[127] = 0;
		Log(LOG_INFO, "ID3v1 MetaData Update: Title='%s' Artist='%s' Genre='%s'",
			gMetaData->strTitle, gMetaData->strArtist, gMetaData->strGenre);
	}
}

void printFrames() //Print each frame from a v2 tag
{
	int x, y, genre, genloc;
	char genNum[4], * gen;

	Log(LOG_INFO, "ID3v2:\n");
	for(x=0;x<numframes;x++)
	{
		if(!strcmp(frames[x].fname, "TCON") && strlen(frames[x].data)<=5)
		{
			for(y=0;y<4;y++) genNum[y]='\0';
			genloc=0;
			for(y=0;y<5;y++)
			{
				if(isdigit(frames[x].data[y]))
				{
					genNum[genloc]=frames[x].data[y];
					genloc++;
				}
			}
			genre=atoi(genNum);
			gen=(char *)calloc(50, sizeof(char));
			getGenre(genre, gen);
			Log(LOG_INFO, "Genre - %s\n", gen);
			if (gMetaData)
			{
				strncpy(gMetaData->strGenre, gen, 128); gMetaData->strGenre[127] = 0;
			}
		}
		else
		{
			Log(LOG_INFO, "%s - %s\n", getFrameName(frames[x].fname), frames[x].data);
			if (gMetaData)
			{
				if (strcmp("TIT2",  frames[x].fname) == 0)
				{
					strncpy(gMetaData->strTitle, frames[x].data, 300); gMetaData->strTitle[299] = 0;
				}
				else if (strcmp("TPE1",  frames[x].fname) == 0)
				{
					strncpy(gMetaData->strArtist, frames[x].data, 300); gMetaData->strArtist[299] = 0;
				}
			}

		}
	}
	
	if (gMetaData)
	{
		Log(LOG_INFO, "ID3v2 MetaData Update: Title='%s' Artist='%s' Genre='%s'",
			gMetaData->strTitle, gMetaData->strArtist, gMetaData->strGenre);
	}
}

void getFrames(char * tag, int tsize) //Grab individual frames from v2 tag
{
	int size, x, loc;
	char fsize[4];
	loc=0;
	while(loc<tsize && numframes<NUMFRAMES && isalpha(tag[loc]))
	{
		for(x=0;x<5;x++) frames[numframes].fname[x]='\0';
		for(x=0;x<FRAMEDATALENGTH;x++) frames[numframes].data[x]='\0';

		memcpy(frames[numframes].fname, tag+loc, 4);
		loc+=4;
		memcpy(fsize, tag+loc, 4);
		loc+=7;
		size=synchsafeToNormal(fsize);
		while(tag[loc]=='\0')//(!isalnum(tag[loc]))
		{
			loc++;
			size--;
		}
		memcpy(frames[numframes++].data, tag+loc, size-1);
		loc+=(size-1);
		//while(tag[loc]=='\0') loc++;
	}
}

int v2(unsigned char * data) //ID3v2
{
	int loc=0;
	unsigned char tagSize[4], flags[1];
	int size;
	char * v2tag;

SEARCH:	while(data[loc]!='I' && loc<len)loc++; //Search for the beginning of a v2 tag
	if(loc>=len) return 0; //Got to end of chunk, no tag
	if(data[++loc]!='D')goto SEARCH;
	if(data[++loc]!='3')goto SEARCH;
	if(data[++loc]>0x04)goto SEARCH;
	if(data[++loc]==0xFF)goto SEARCH;
	loc++;
	if(data[++loc]>=0x80)goto SEARCH;
	if(data[++loc]>=0x80)goto SEARCH;
	if(data[++loc]>=0x80)goto SEARCH;
	if(data[++loc]>=0x80)goto SEARCH;
	if(loc>=len) {Log(LOG_INFO, "Failed\n"); return 0;}
	loc++;

	memcpy(tagSize, data+loc-4, 4); //Grab tag size and flags
	memcpy(flags, data+loc-5, 1);

	if(GETBIT(flags, 1)) //This bit flags an extended header
	{
		Log(LOG_INFO, "I haven't implemented extended headers.  Sorry.\n");
		return 0; //I don't want to deal with extended headers yet
	}
	if(GETBIT(flags, 3)) //This bit flags a footer
	{
		Log(LOG_INFO, "I haven't implemented footers.  Sorry.\n");
		return 0; //I don't want to deal with footers yet
	}

	size=synchsafeToNormal((char*)tagSize); //Get the size of the entire tag, excluding the header

	v2tag=(char *)calloc(size+1, sizeof(char));
	memcpy(v2tag, data+loc, size);

	getFrames(v2tag, size);
	printFrames();

	return 1;
}

void v1() //ID3v1
{
	int loc=0;
	if(!memcmp(cFileEnd+loc, "TAG", 3))	//Marks a
	{							//v1 tag
		loc+=3;
		memcpy(info[0], cFileEnd+loc, 30); //Title
		memcpy(info[1], cFileEnd+loc+30, 30); //Artist
		memcpy(info[2], cFileEnd+loc+60, 30); //Album
		memcpy(info[3], cFileEnd+loc+90, 4); //Year
		memcpy(info[4], cFileEnd+loc+94, 30); //Comment
		memcpy(&gen, cFileEnd+loc+124, 1); //Genre
		printData();//"ID3V1");
	}
}

/*
int main(int argc, char *argv[])
{
	int x;
	if(argc==1)
	{
		Log(LOG_INFO, "ID3 Viewer\n\nYou must specify a filename.\n");
		return 1;
	}
	readFileEnd(argv[1]); //v2 tags can be at the beginning or end of the file,
	readFileBegin(argv[1]);//so check both
	numframes=0;

	for(x=0;x<5;x++)//allocate Data locations based on v1 tag size
	{
		if(x<3 || x==4) info[x]=(char *)calloc(31, sizeof(char));
		if(x==3) info[x]=(char *)calloc(5, sizeof(char));
	}
	
	v1();
	v2(cFileBegin);

	for(x=0;x<5;x++)
			free(info[x]);
	return 1;
}
*/

int GetID3Data(char *Filename, MetaData *MetaData)
{
	int x;
	
	Log(LOG_INFO, "ID3 Parsing on '%s'", Filename);
	
	gMetaData = MetaData;
	
	readFileEnd(Filename); //v2 tags can be at the beginning or end of the file,
	readFileBegin(Filename);//so check both
	numframes=0;

	for(x=0;x<5;x++)//allocate Data locations based on v1 tag size
	{
		if(x<3 || x==4) info[x]=(char *)calloc(31, sizeof(char));
		if(x==3) info[x]=(char *)calloc(5, sizeof(char));
	}
	
	v1();
	v2(cFileBegin);

	for(x=0;x<5;x++)
			free(info[x]);
			
	gMetaData = NULL;
			
	return 1;
}






//These are helper functions, that just turn codes into data

char * getFrameName(char fname[])//I got a list of frames from the internet, and just
{								//inserted it here
	if(!strcmp(fname, "AENC"))
		return "Audio encryption";
	if(!strcmp(fname, "APIC"))
		return "Attached picture";
	if(!strcmp(fname, "ASPI"))
		return "Audio seek point index";
	if(!strcmp(fname, "COMM"))
		return "Comments";
	if(!strcmp(fname, "COMR"))
		return "Commercial frame";
	if(!strcmp(fname, "ENCR"))
		return "Encryption method registration";
	if(!strcmp(fname, "EQU2"))
		return "Equalisation (2)";
	if(!strcmp(fname, "ETCO"))
		return "Event timing codes";
	if(!strcmp(fname, "GEOB"))
		return "General encapsulated object";
	if(!strcmp(fname, "GRID"))
		return "Group identification registration";
	if(!strcmp(fname, "LINK"))
		return "Linked information";
	if(!strcmp(fname, "MCDI"))
		return "Music CD identifier";
	if(!strcmp(fname, "MLLT"))
		return "MPEG location lookup table";
	if(!strcmp(fname, "OWNE"))
		return "Ownership frame";
	if(!strcmp(fname, "PRIV"))
		return "Private frame";
	if(!strcmp(fname, "PCNT"))
		return "Play counter";
	if(!strcmp(fname, "POPM"))
		return "Popularimeter";
	if(!strcmp(fname, "POSS"))
		return "Position synchronisation frame";
	if(!strcmp(fname, "RBUF"))
		return "Recommended buffer size";
	if(!strcmp(fname, "RVA2"))
		return "Relative volume adjustment (2)";
	if(!strcmp(fname, "RVRB"))
		return "Reverb";
	if(!strcmp(fname, "SEEK"))
		return "Seek frame";
	if(!strcmp(fname, "SIGN"))
		return "Signature frame";
	if(!strcmp(fname, "SYLT"))
		return "Synchronised lyric/text";
	if(!strcmp(fname, "SYTC"))
		return "Synchronised tempo codes";
	if(!strcmp(fname, "TALB"))
		return "Album";
	if(!strcmp(fname, "TBPM"))
		return "BPM (beats per minute)";
	if(!strcmp(fname, "TCOM"))
		return "Composer";
	if(!strcmp(fname, "TCON"))
		return "Content type";
	if(!strcmp(fname, "TCOP"))
		return "Copyright message";
	if(!strcmp(fname, "TDEN"))
		return "Encoding time";
	if(!strcmp(fname, "TDLY"))
		return "Playlist delay";
	if(!strcmp(fname, "TDOR"))
		return "Original release time";
	if(!strcmp(fname, "TDRC"))
		return "Recording time";
	if(!strcmp(fname, "TDRL"))
		return "Release time";
	if(!strcmp(fname, "TDTG"))
		return "Tagging time";
	if(!strcmp(fname, "TENC"))
		return "Encoded by";
	if(!strcmp(fname, "TEXT"))
		return "Lyricist/Text writer";
	if(!strcmp(fname, "TFLT"))
		return "File type";
	if(!strcmp(fname, "TIPL"))
		return "Involved people list";
	if(!strcmp(fname, "TIT1"))
		return "Content group description";
	if(!strcmp(fname, "TIT2"))
		return "Title";
	if(!strcmp(fname, "TIT3"))
		return "Subtitle/Description refinement";
	if(!strcmp(fname, "TKEY"))
		return "Initial key";
	if(!strcmp(fname, "TLAN"))
		return "Language(s)";
	if(!strcmp(fname, "TLEN"))
		return "Length";
	if(!strcmp(fname, "TMCL"))
		return "Musician credits list";
	if(!strcmp(fname, "TMED"))
		return "Media type";
	if(!strcmp(fname, "TMOO"))
		return "Mood";
	if(!strcmp(fname, "TOAL"))
		return "Original album/movie/show title";
	if(!strcmp(fname, "TOFN"))
		return "Original filename";
	if(!strcmp(fname, "TOLY"))
		return "Original lyricist(s)/text writer(s)";
	if(!strcmp(fname, "TOPE"))
		return "Original artist(s)/performer(s)";
	if(!strcmp(fname, "TOWN"))
		return "File owner/licensee";
	if(!strcmp(fname, "TPE1"))
		return "Lead performer(s)";
	if(!strcmp(fname, "TPE2"))
		return "Band/orchestra/accompaniment";
	if(!strcmp(fname, "TPE3"))
		return "Conductor/performer refinement";
	if(!strcmp(fname, "TPE4"))
		return "Interpreted, remixed, or otherwise modified by";
	if(!strcmp(fname, "TPOS"))
		return "Part of a set";
	if(!strcmp(fname, "TPRO"))
		return "Produced notice";
	if(!strcmp(fname, "TPUB"))
		return "Publisher";
	if(!strcmp(fname, "TRCK"))
		return "Track";
	if(!strcmp(fname, "TRSN"))
		return "Internet radio station name";
	if(!strcmp(fname, "TRSO"))
		return "Internet radio station owner";
	if(!strcmp(fname, "TSOA"))
		return "Album sort order";
	if(!strcmp(fname, "TSOP"))
		return "Performer sort order";
	if(!strcmp(fname, "TSOT"))
		return "Title sort order";
	if(!strcmp(fname, "TSRC"))
		return "ISRC (international standard recording code)";
	if(!strcmp(fname, "TSSE"))
		return "Software/Hardware and settings used for encoding";
	if(!strcmp(fname, "TSST"))
		return "Set subtitle";
	if(!strcmp(fname, "TYER"))
		return "Year";
	if(!strcmp(fname, "UFID"))
		return "Unique file identifier";
	if(!strcmp(fname, "USER"))
		return "Terms of use";
	if(!strcmp(fname, "USLT"))
		return "Unsynchronised lyric/text transcription";
	if(!strcmp(fname, "WCOM"))
		return "Commercial information";
	if(!strcmp(fname, "WCOP"))
		return "Copyright/Legal information";
	if(!strcmp(fname, "WOAF"))
		return "Official audio file webpage";
	if(!strcmp(fname, "WOAR"))
		return "Official artist/performer webpage";
	if(!strcmp(fname, "WOAS"))
		return "Official audio source webpage";
	if(!strcmp(fname, "WORS"))
		return "Official Internet radio station homepage";
	if(!strcmp(fname, "WPAY"))
		return "Payment";
	if(!strcmp(fname, "WPUB"))
		return "Publishers official webpage";
	return "Undefined";
}

void getGenre(int val, char * out) //I found a list online that matches genre numbers
{								   //with the right string
	switch (val)
	{
	case 1:
		strcpy(out, "Blues");
		break;
	case 2:
		strcpy(out, "Classic Rock");
		break;
	case 3:
		strcpy(out, "Country ");
		break;
	case 4:
		strcpy(out, "Dance");
		break;
	case 5:
		strcpy(out, "Disco");
		break;
	case 6:
		strcpy(out, "Funk ");
		break;
	case 7:
		strcpy(out, "Grunge");
		break;
	case 8:
		strcpy(out, "Hip-Hop ");
		break;
	case 9:
		strcpy(out, "Jazz ");
		break;
	case 10:
		strcpy(out, "Metal");
		break;
	case 11:
		strcpy(out, "New Age ");
		break;
	case 12:
		strcpy(out, "Oldies");
		break;
	case 13:
		strcpy(out, "Other");
		break;
	case 14:
		strcpy(out, "Pop ");
		break;
	case 15:
		strcpy(out, "R&B ");
		break;
	case 16:
		strcpy(out, "Rap ");
		break;
	case 17:
		strcpy(out, "Reggae");
		break;
	case 18:
		strcpy(out, "Rock");
		break;
	case 19:
		strcpy(out, "Techno");
		break;
	case 20:
		strcpy(out, "Industrial");
		break;
	case 21:
		strcpy(out, "Alternative");
		break;
	case 22:
		strcpy(out, "Ska");
		break;
	case 23:
		strcpy(out, "Death Metal");
		break;
	case 24:
		strcpy(out, "Pranks");
		break;
	case 25:
		strcpy(out, "Soundtrack");
		break;
	case 26:
		strcpy(out, "Euro-Techno");
		break;
	case 27:
		strcpy(out, "Ambient");
		break;
	case 28:
		strcpy(out, "Trip-Hop");
		break;
	case 29:
		strcpy(out, "Vocal");
		break;
	case 30:
		strcpy(out, "Jazz+Funk");
		break;
	case 31:
		strcpy(out, "Fusion");
		break;
	case 32:
		strcpy(out, "Trance");
		break;
	case 33:
		strcpy(out, "Classical");
		break;
	case 34:
		strcpy(out, "Instrumental");
		break;
	case 35:
		strcpy(out, "Acid");
		break;
	case 36:
		strcpy(out, "House");
		break;
	case 37:
		strcpy(out, "Game");
		break;
	case 38:
		strcpy(out, "Sound Clip");
		break;
	case 39:
		strcpy(out, "Gospel");
		break;
	case 40:
		strcpy(out, "Noise");
		break;
	case 41:
		strcpy(out, "AlternRock");
		break;
	case 42:
		strcpy(out, "Bass");
		break;
	case 43:
		strcpy(out, "Soul");
		break;
	case 44:
		strcpy(out, "Punk");
		break;
	case 45:
		strcpy(out, "Space");
		break;
	case 46:
		strcpy(out, "Meditative");
		break;
	case 47:
		strcpy(out, "Instrumental Pop");
		break;
	case 48:
		strcpy(out, "Instrumental Rock");
		break;
	case 49:
		strcpy(out, "Ethnic");
		break;
	case 50:
		strcpy(out, "Gothic");
		break;
	case 51:
		strcpy(out, "Darkwave");
		break;
	case 52:
		strcpy(out, "Techno-Industrial");
		break;
	case 53:
		strcpy(out, "Electronic");
		break;
	case 54:
		strcpy(out, "Pop-Folk");
		break;
	case 55:
		strcpy(out, "Eurodance");
		break;
	case 56:
		strcpy(out, "Dream");
		break;
	case 57:
		strcpy(out, "Southern Rock");
		break;
	case 58:
		strcpy(out, "Comedy");
		break;
	case 59:
		strcpy(out, "Cult");
		break;
	case 60:
		strcpy(out, "Gangsta");
		break;
	case 61:
		strcpy(out, "Top 40");
		break;
	case 62:
		strcpy(out, "Christian Rap");
		break;
	case 63:
		strcpy(out, "Pop/Funk");
		break;
	case 64:
		strcpy(out, "Jungle");
		break;
	case 65:
		strcpy(out, "Native American");
		break;
	case 66:
		strcpy(out, "Cabaret");
		break;
	case 67:
		strcpy(out, "New Wave");
		break;
	case 68:
		strcpy(out, "Psychadelic");
		break;
	case 69:
		strcpy(out, "Rave");
		break;
	case 70:
		strcpy(out, "Showtunes");
		break;
	case 71:
		strcpy(out, "Trailer");
		break;
	case 72:
		strcpy(out, "Lo-Fi");
		break;
	case 73:
		strcpy(out, "Tribal");
		break;
	case 74:
		strcpy(out, "Acid Punk");
		break;
	case 75:
		strcpy(out, "Acid Jazz");
		break;
	case 76:
		strcpy(out, "Polka");
		break;
	case 77:
		strcpy(out, "Retro");
		break;
	case 78:
		strcpy(out, "Musical");
		break;
	case 79:
		strcpy(out, "Rock & Roll");
		break;
	case 80:
		strcpy(out, "Hard Rock");
		break;
	case 81:
		strcpy(out, "Folk");
		break;
	case 82:
		strcpy(out, "Folk/Rock ");
		break;
	case 83:
		strcpy(out, "National Folk");
		break;
	case 84:
		strcpy(out, "Swing");
		break;
	case 85:
		strcpy(out, "Bebob");
		break;
	case 86:
		strcpy(out, "Latin");
		break;
	case 87:
		strcpy(out, "Revival");
		break;
	case 88:
		strcpy(out, "Celtic");
		break;
	case 89:
		strcpy(out, "Bluegrass");
		break;
	case 90:
		strcpy(out, "Avantgarde");
		break;
	case 91:
		strcpy(out, "Gothic Rock");
		break;
	case 92:
		strcpy(out, "Progressive Rock");
		break;
	case 93:
		strcpy(out, "Psychedelic Rock");
		break;
	case 94:
		strcpy(out, "Symphonic Rock");
		break;
	case 95:
		strcpy(out, "Slow Rock");
		break;
	case 96:
		strcpy(out, "Big Band");
		break;
	case 97:
		strcpy(out, "Chorus");
		break;
	case 98:
		strcpy(out, "Easy Listening");
		break;
	case 99:
		strcpy(out, "Acoustic");
		break;
	case 100:
		strcpy(out, "Humour");
		break;
	case 101:
		strcpy(out, "Speech");
		break;
	case 102:
		strcpy(out, "Chanson");
		break;
	case 103:
		strcpy(out, "Opera");
		break;
	case 104:
		strcpy(out, "Chamber Music");
		break;
	case 105:
		strcpy(out, "Sonata");
		break;
	case 106:
		strcpy(out, "Symphony");
		break;
	case 107:
		strcpy(out, "Booty Bass");
		break;
	case 108:
		strcpy(out, "Primus");
		break;
	case 109:
		strcpy(out, "Porn Groove");
		break;
	case 110:
		strcpy(out, "Satire");
		break;
	case 111:
		strcpy(out, "Slow Jam");
		break;
	case 112:
		strcpy(out, "Club");
		break;
	case 113:
		strcpy(out, "Tango");
		break;
	case 114:
		strcpy(out, "Samba");
		break;
	case 115:
		strcpy(out, "Folklore");
		break;
	default:
		strcpy(out, "Not Specified");
		break;
	}
}
