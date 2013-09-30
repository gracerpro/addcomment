/*

*/

#include "global.h"
#include <string.h>
#include <io.h>

bool fileExists(const char *szFile) {
#ifdef WIN32
	return access(szFile, 0) == 0;
#else
	return std::iostream::good(szFile); // TODO: debug
#endif
}

char* deleteQuotes(char *szFileName) {
	if (!szFileName || !szFileName[0])
		return szFileName;

	size_t len = strlen(szFileName);
	if (szFileName[0] == '"' || szFileName[0] == '\'') {
		memmove(&szFileName[0], &szFileName[1], len - 1);
		--len;
	}
	if (len > 1 && (szFileName[len - 1] == '"' || szFileName[len - 1] == '\'')) {
		szFileName[len - 1] = 0;
	}

	return szFileName;
}