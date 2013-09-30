/*
 * global.h
 */

#define SZ_EXE_NAME           "addcomment"

#ifdef WIN32
	#define SZ_EXE_FILE_NAME  "addcomment.exe"
	#define DIR_SEP           "\\"
	#define DIR_SEP_CHAR      '\\'
#else
	#define SZ_EXE_FILE_NAME  "addcomment"
	#define DIR_SEP           "/"
	#define DIR_SEP_CHAR      '/'
#endif


bool fileExists(const char *szFile);
char* deleteQuotes(char *szFileName);