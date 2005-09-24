#ifndef _CLOGGINGH_
#define _CLOGGINGH_

enum loglevel_enum
{
	LOG_LOWLEVEL = 10,
	LOG_INFO	 = 50,
	LOG_ERROR	 = 80,
	LOG_ALWAYS	 = 100
};

class CLogging
{
public:
	CLogging();
	~CLogging();

	int Set(char *strLogFilename, loglevel_enum iLogLevel);
	void SetLevel(loglevel_enum iNewLevel);
	
	int Log(char *strModuleName, loglevel_enum LogLevel, char *strFormat, ...);
	
private:
	char *m_strFilename;
	loglevel_enum m_LogLevel;
	
};

#endif
