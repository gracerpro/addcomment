/*
 * global.h
 */

#define SZ_EXE_NAME           "addcomment"

#ifdef _WIN32
	#define SZ_EXE_FILE_NAME  "addcomment.exe"
#else
	#define SZ_EXE_FILE_NAME  "addcomment"
#endif


#if defined(_WIN32)
	#define PATH_SEPARATOR      "\\"
	#define PATH_SEPARATOR_CHAR '\\'
#else
	#define PATH_SEPARATOR      "/"
	#define PATH_SEPARATOR_CHAR '/'
#endif


const char*   getWorkDir();
const char*   getExeDir();
char*         getPathSepEnd(char *szDir);
bool          fileExists(const char *szFile);
bool          isNewline(char c);
bool          isPathSep(char c);
bool          isAbsoluteFilePath(const char *szFileName);
char*         setAbsolutePath(char *szFile);
