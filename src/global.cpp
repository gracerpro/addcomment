/*

*/
#include "global.h"
#include <string.h>
//#include <sys\stat.h>
//#include <sys\types.h>
#ifdef _WIN32
	#include <io.h>
	#include <Windows.h>
#else
	#include <dirent.h>
#endif

bool fileExists(const char *szFile) {
#ifdef WIN32
	return access(szFile, 0) == 0;
#else
	return std::iostream::good(szFile);
#endif
}

bool isPathSep(char c) {
	return (c =='/' || c == '\\');
}

bool isNewline(char c) {
	return (c == '\r' || c == '\n');
}

char* getPathSepEnd(char *szDir) {
	char *p = strrchr(szDir, '/');
	if (!p)
		p = strrchr(szDir, '\\');
	if (!p)
		return NULL;

	return p + 1;
}

const char* getWorkDir() {
	static char buf[MAX_PATH];

	DWORD size = GetCurrentDirectory(MAX_PATH, buf);
	char *p = &buf[size - 1];
	if (!isPathSep(*p)) {
		*(++p) = PATH_SEPARATOR_CHAR;
	}
	*(++p) = 0;

	return buf;
}

const char* getExeDir() {
	static char buf[MAX_PATH];

	GetModuleFileName(NULL, buf, MAX_PATH);
	char *p = strrchr(buf, PATH_SEPARATOR_CHAR);
	*(p + 1) = 0;

	return buf;
}

bool isAbsoluteFilePath(const char *szFileName) {
#ifdef _WIN32
	return strchr(szFileName, ':') != NULL;
#else
	char *p = strchr(szFileName, '/'); // TODO: debug
	if (p) {
		if (p > szFileName) {
			return *(p - 1) != '.';
		}
	}
	return false;
#endif
}

char* setAbsolutePath(char *szFile) {
	if (!isAbsoluteFilePath(szFile)) {
		const char *dir = getWorkDir();
		size_t dirLen = strlen(dir);
		memmove(szFile + dirLen, szFile, strlen(szFile) + 1);
		memmove(szFile, dir, dirLen);
	}

	return szFile;
}

bool makeFileHidden(const char *szFile) {
#ifdef _WIN32
	return SetFileAttributes(szFile, FILE_ATTRIBUTE_HIDDEN) > 0;
#endif
}
