#include "global.h"
#include "file_handle.h"
#include <conio.h>
#include <iostream>
#include <fstream>
#include <string>
#ifdef _WIN32
	#include <Windows.h>
#endif

using namespace std;

inline bool isParentDirRef(const char* szFileName) {
	return (szFileName[0] == '.' && szFileName[1] == '.' && szFileName[2] == 0);
}

inline bool isCurDirRef(const char* szFileName) {
	return (szFileName[0] == '.' && szFileName == 0);
}

void showHelp() {
	cout << SZ_EXE_FILE_NAME " \"path_to_target_direcrory\" [\"comment_file_name\"] "
		"[\"filter_str\"] " << endl;

	cout << SZ_EXE_FILE_NAME " -dir path_to_direcrory [-c FILE] "
		"[-f[ilter] file_filter] " << endl;

	cout << "[-b]\t(buckup)" << endl;
}

char* seekComment(char *data) {
	char *p = data;

	if (*p != '/')
		return p;

	while (*p) {
		if (*p == '/' && p[1] == '/') {
			while (*p && !isNewline(*p))
				++data;
			while (isNewline(*p))
				++data;
			continue;
		}
		if (*p == '/' && p[1] == '*') {
			p += 2;
			while (*p && !(*p == '*' && p[1] == '/'))
				++p;
			if (p) {
				p += 2;
				while (isNewline(*p))
					++p;
			}
			continue;
		}
		break;
	}

	while (*p && (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t'))
		++p;

	return p;
}

char* addFilterToDir(char *szTargetDir, const char *szFilter) {
	char *p = &szTargetDir[strlen(szTargetDir) - 1];

	if (!isPathSep(*p)) {
		*(++p) = PATH_SEPARATOR_CHAR;
		*(++p) = 0;
	}
	else {
		p = getPathSepEnd(szTargetDir);
		if (!p)
			return NULL;
		*p = 0;
	}

	(szFilter) ? strcat(p, szFilter) : strcat(p, "*.*");

	return szTargetDir;
}

bool isWinNewLine(const char *szFilePath) {
	char buf[201];
	bool res = true;

	std::fstream file(szFilePath, std::ifstream::binary | std::ifstream::in);

	if (file.bad())
		return res;

	buf[200] = 0;
	while (!file.eof()) {
		buf[0] = 0;
		file.read(buf, 200);
		if (!buf[0])
			break;
		char *p = buf;
		while (*p && *p != '\n')
			++p;
		if (p > buf && p[-1] == '\r') {
			res = true;
			break;
		}
		if (*p == '\n') {
			res = false;
			break;
		}
	}
	file.close();

	return res;
}

ptrdiff_t addCommentToFile(const char *szFilePath, const char *szComment, const bool bBackup) {
	std::fstream fs(szFilePath, std::ifstream::binary | std::fstream::in);

	if (fs.fail()) {
		cerr << "Could not open file " << szFilePath << endl;
		return -1;
	}

	fs.seekp(0, std::ios::end);
	long size = static_cast<long>(fs.tellp());
	fs.seekp(0, std::ios::beg);

	char *pData = NULL;
	try
	{
		pData = new char[size + 1];
	}
	catch (...)
	{
		cerr << "Memory allocation failed. Size " << size << endl;
		fs.close();
		return 0;
	}

	fs.read(pData, size);
	pData[size] = 0;
	fs.close();

	char* pCodeData = seekComment(pData);

	if (bBackup) {
		std::string sFile(szFilePath);
		sFile += ".bak";

		std::ofstream ofs;
		ofs.open(sFile.data(), std::ios_base::out | std::ios_base::trunc);
		if (ofs.good()) {
			ofs.write(pData, size);
			ofs.close();
			makeFileHidden(sFile.c_str());
		}
	}

	fs.open(szFilePath, std::fstream::out | std::ifstream::binary | std::ofstream::trunc);
	fs << szComment;
	fs.write(pCodeData, size - (pCodeData - pData));
	fs.close();

	delete[] pData;

	return pCodeData - pData;
}

size_t addCommentToDir(char *szTargetDir, const char *szCommentWin, const char *szCommentUnix,
	const bool bBackup, const char *szFilter)
{
	addFilterToDir(szTargetDir, szFilter);

	FileHandler fileHandler;
	size_t count = 0;

	cout << "DIR: " << szTargetDir << endl;

	if (!fileHandler.getFirstFile(szTargetDir))
		return 0;

	bool bFilterAll = strcmp(szFilter, "*.*") == 0;
	char *pEnd = getPathSepEnd(szTargetDir);

	do
	{
		const char *szFile = fileHandler.getFileName();
		if (isParentDirRef(szFile) || isCurDirRef(szFile)) {
			continue;
		}

		if (fileHandler.isDirectory()) {
			strcpy(pEnd, fileHandler.getFileName());
			strcat(pEnd, PATH_SEPARATOR);
			addCommentToDir(szTargetDir, szCommentWin, szCommentUnix, bBackup, szFilter);
			continue;
		}

		if (!fileHandler.isWriteableFile()) {
			continue;
		}

		strcpy(pEnd, fileHandler.getFileName());
		cout << fileHandler.getFileName() << endl;

		const char *szComment = (isWinNewLine(szTargetDir)) ? szCommentWin : szCommentUnix;

		addCommentToFile(szTargetDir, szComment, bBackup);
		++count;
	}
	while (fileHandler.getNextFile());

	fileHandler.close();

	*pEnd = 0;
	// loop throuth directories
	if (!bFilterAll) {
		addFilterToDir(szTargetDir, "*.*");
		if (fileHandler.getFirstFile(szTargetDir)) {
			while (fileHandler.getNextFile()) {
				const char *szFile = fileHandler.getFileName();
                if (isParentDirRef(szFile) || isCurDirRef(szFile))
                    continue;

				if (fileHandler.isDirectory()) {
					strcpy(pEnd, fileHandler.getFileName());
					strcat(pEnd, PATH_SEPARATOR);
					addCommentToDir(szTargetDir, szCommentWin, szCommentUnix, bBackup, szFilter);
					*pEnd = 0;
				}
			}
		}
		fileHandler.close();
		*pEnd = 0;
	}

	return count;
}

bool readComment(const char *szCommentFile, std::string &sCommentWin, std::string& sCommentUnix) {
	std::ifstream ifs(szCommentFile, std::ifstream::binary);

	if (ifs.fail())
		return false;

	if (isWinNewLine(szCommentFile)) {
		// Dos to Unix convert
		sCommentWin.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
		size_t len = sCommentWin.size();
		char *buffer = new char[len];
		char *p = buffer;
		const char *sz = sCommentWin.c_str();

		for (size_t i = 0; i < len; ++i) {
			if (sz[i] == '\r')
				continue;
			*p = sz[i];
			++p;
		}
		*p = 0;
		sCommentUnix.assign(buffer);
	}
	else {
		// Unix to Dos convert
		sCommentUnix.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
		size_t len = sCommentWin.size();
		char *buffer = new char[len * 2];
		char *p = buffer;
		const char *sz = sCommentWin.c_str();

		for (size_t i = 0; i < len; ++i) {
			if (sz[i] == '\r')
				continue;
			if (sz[i] == '\n') {
				*p++ = '\r';
			}
			*p = sz[i];
			++p;
		}
		*p = 0;
		sCommentWin.assign(buffer);
	}

	ifs.close();

	return true;
}

int parseArguments(const int argc, const char **argv, char *szTargetDir,
	char *szCommentFile, std::string &filter, bool &bBackup)
{
	int count = 0;

	bBackup = false;
	filter  = "";
	szCommentFile[0] = 0;
	szTargetDir[0] = 0;
	for (int i = 1; i < argc; ++i) {
		const char *pStart = argv[i];

		if (!strcmp(pStart, "-dir")) {
			if (++i < argc) {
				strcpy(szTargetDir, argv[i]);
				setAbsolutePath(szTargetDir);
				++count;
			}
		}
		else if (!strcmp(pStart, "-c")) {
			if (++i < argc) {
				strcpy(szCommentFile, argv[i]);
				setAbsolutePath(szCommentFile);
				++count;
			}
		}
		else if (!strcmp(pStart, "-b")) {
			bBackup = true;
		}
		else if (!strcmp(pStart, "-f")) {
			if (++i < argc) {
				filter = argv[i];
				++count;
			}
		}
		else if (!strcmp(pStart, "-filter")) {
			if (++i < argc) {
				filter = argv[i];
				++count;
			}
		}
	}

	if (!szTargetDir[0] && argc >= 1) {
		strcpy(szTargetDir, argv[1]);
		setAbsolutePath(szTargetDir);
	}
	if (!szCommentFile[0] && argc > 2) {
		strcpy(szCommentFile, argv[2]);
		setAbsolutePath(szCommentFile);
	}

	return count;
}

int main(int argc, const char **argv) {
	if (argc == 1) {
		showHelp();
		return 0;
	}

	char         szTargetDir[MAX_PATH];
	char         szCommentFile[MAX_PATH];
	std::string  sCommentWin;
	std::string  sCommentUnix;
	std::string  sFilter;
	bool         bBackup;

	parseArguments(argc, argv, szTargetDir, szCommentFile, sFilter, bBackup);

	if (!szTargetDir[0]) {
		showHelp();
		cerr << "The target directory is not found" << endl;
		return -1;
	}

	char *pEnd = getPathSepEnd(szTargetDir);
	if (!pEnd) {
		size_t len = strlen(szTargetDir);
		szTargetDir[len]     = PATH_SEPARATOR_CHAR;
		szTargetDir[len + 1] = 0;
	}

	if (!readComment(szCommentFile, sCommentWin, sCommentUnix)) {
		cerr << "Couldn't read the comment file <" << szCommentFile << ">" << endl;
		return -1;
	}

	if (sFilter.empty()) {
		sFilter = "*.*";
	}

	cout << "In \"" << szTargetDir << "\" directory will be change all files by filter " << sFilter << endl;
	cout << "Press any key to continue. Press \"Esc\" to exit." << endl;

	char cExit = static_cast<char>(getch());
	if (cExit == 27) // Escape key
		return 0;

	char szFilter[200];
	const char *pFilter = sFilter.data();
	while (const char *p = strchr(pFilter, ';'))
	{
		size_t len = p - pFilter;
		strncpy(szFilter, pFilter, len);
		szFilter[len] = 0;
		addCommentToDir(szTargetDir, sCommentWin.c_str(), sCommentUnix.c_str(), bBackup, szFilter);
		pFilter += len + 1;
	}
	if (pFilter && pFilter[0])
		addCommentToDir(szTargetDir, sCommentWin.c_str(), sCommentUnix.c_str(), bBackup, pFilter);

	return 0;
}
