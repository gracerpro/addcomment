/*

*/
#include <conio.h>
#include <iostream>
#include <fstream>
#include <string>
#include "global.h"
#ifdef WIN32
	#include <Windows.h>
#endif

using namespace std;

void showHelp() {
	cout << SZ_EXE_FILE_NAME " \"path_to_target_direcrory\" [\"comment_file_name\"] "
		"[\"filter_str\"] " << endl;

	cout << SZ_EXE_FILE_NAME " -target_dir:path_to_direcrory [-c[omment_file]:file] "
		"[-filter:file_filter] " << endl;
}

inline bool isNewline(char c) {
	return (c == 0x0A || c == 0x0D);
}

inline bool isDirSep(char c) {
	return (c =='/' || c == '\\');
}

char* getDirSepEnd(char *szDir) {
	char *p = strrchr(szDir, '/');
	if (!p)
		p = strrchr(szDir, '\\');
	if (!p)
		return NULL;

	return p + 1;
}

char* seekComment(char *data) {
	if (data[0] != '/')
		return data;

	while (data[0]) {
		if (data[0] == '/' && data[1] == '/') {
			while (data[0] && !isNewline(data[0]))
				++data;
			while (isNewline(data[0]))
				++data;
			continue;
		}
		if (data[0] == '/' && data[1] == '*') {
			data += 2;
			while (data[0] && !(data[0] == '*' && data[1] == '/'))
				++data;
			if (data[0]) {
				data += 2;
				while (isNewline(data[0]))
					++data;
			}
			continue;
		}
		break;
	}

	return data;
}

char* addFilterToDir(char *szTargetDir, const char *szFilter) {
	char *p = &szTargetDir[strlen(szTargetDir) - 1];

	if (!(*p == '/' || *p == '\\')) {
		*(++p) = DIR_SEP_CHAR;
		*(++p) = 0;
	}
	else {
		p = getDirSepEnd(szTargetDir);
		if (!p)
			return NULL;
		*p = 0;
	}

	(szFilter) ? strcat(p, szFilter) : strcat(p, "*.*");

	return szTargetDir;
}

int addCommentToFile(const char *szFilePath, const std::string &sComment, const bool bBackup) {
	std::fstream fs(szFilePath, std::ifstream::binary | std::fstream::in);

	if (fs.fail()) {
		cerr << "Could not open file " << szFilePath << endl;
		return -1;
	}

	fs.seekp(0, std::ios::end);
	long size = static_cast<long>(fs.tellp());
	fs.seekp(0, std::ios::beg);

	char *pData = new char[size + 1];

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
			SetFileAttributes(sFile.c_str(), FILE_ATTRIBUTE_HIDDEN);
		}
	}

	fs.open(szFilePath, std::fstream::out | std::ifstream::binary | std::ofstream::trunc);
	fs.write(sComment.c_str(), sComment.length());
	fs.write(pCodeData, size - (pCodeData - pData));
	fs.close();

	delete[] pData;

	return pCodeData - pData + sComment.length();
}

int addCommentToDir(char *szTargetDir, const std::string &sComment, const bool bBackup,
	const char *szFilter) {

	addFilterToDir(szTargetDir, szFilter);

	WIN32_FIND_DATA wfd = {0};
	size_t count = 0;

	cout << "DIR: " << szTargetDir << endl;

	HANDLE hFind = FindFirstFile(szTargetDir, &wfd);
	if (hFind == INVALID_HANDLE_VALUE)
		return 0;

	bool bFilterAll = strcmp(szFilter, "*.*") == 0;
	char *pEnd = getDirSepEnd(szTargetDir);

	do
	{
	    const char *szFile = wfd.cFileName;
		if ((szFile[0] == '.' && szFile[1] == '.' && szFile[2] == 0) ||
			(szFile[0] == '.' && szFile[1] == 0))
			continue;

		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			strcpy(pEnd, wfd.cFileName);
			strcat(pEnd, DIR_SEP);
			addCommentToDir(szTargetDir, sComment, bBackup, szFilter);
			continue;
		}

		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ||
			wfd.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE ||
			wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY ||
			wfd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
		{
			continue;
		}

		strcpy(pEnd, wfd.cFileName);
		cout << wfd.cFileName << endl;
		addCommentToFile(szTargetDir, sComment, bBackup);
		++count;
	}
	while (FindNextFile(hFind, &wfd));

	FindClose(hFind);

	*pEnd = 0;
	// loop throuth directories
	if (!bFilterAll) {
		addFilterToDir(szTargetDir, "*.*");
		HANDLE hFind = FindFirstFile(szTargetDir, &wfd);
		if (hFind) {
			while (FindNextFile(hFind, &wfd)) {
				const char *szFile = wfd.cFileName;
                if ((szFile[0] == '.' && szFile[1] == '.' && szFile[2] == 0) ||
                    (szFile[0] == '.' && szFile[1] == 0))
                    continue;
				if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					strcpy(pEnd, wfd.cFileName);
					strcat(pEnd, DIR_SEP);
					addCommentToDir(szTargetDir, sComment, bBackup, szFilter);
					*pEnd = 0;
				}
			}
		}
	}

	return count;
}

bool readComment(const char *szCommentFile, std::string &sComment) {
	std::ifstream ifs(szCommentFile, ios::out);

	if (ifs.fail())
		return false;

	sComment.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
	ifs.close();

	return true;
}

int parsingArguments(const int argc, const char **argv, char *szTargetDir,
	char *szCommentFile, std::string &filter, bool &bBackup) {
	int count = 0;

	bBackup = false;
	filter  = "";
	szCommentFile[0] = 0;
	szTargetDir[0] = 0;
	for (int i = 1; i < argc; ++i) {
		const char *pStart = argv[i];

		if (!strncmp(pStart, "-t:", 3)) {
			if (fileExists(pStart + 3)) {
				strcpy(szTargetDir, pStart + 3);
				deleteQuotes(szTargetDir);
				++count;
			}
		}
		else if (!strncmp(pStart, "-target_dir:", 12)) {
			strcpy(szTargetDir, pStart + 12);
			deleteQuotes(szTargetDir);
			++count;
		}
		else if (!strncmp(pStart, "-c:", 3)) {
				strcpy(szCommentFile, pStart + 3);
				deleteQuotes(szCommentFile);
				if (!fileExists(szCommentFile)) {
					std::string str = argv[0];
					size_t index = str.rfind(DIR_SEP_CHAR) + 1;
					str.erase(str.begin() + index, str.end());
					str += szCommentFile;
					strcpy(szCommentFile, str.data());
				}
				++count;
		}
		else if (!strcmp(pStart, "-b")) {
			bBackup = true;
		}
		else if (!strncmp(pStart, "-f:", 3)) {
			filter = pStart + 3;
		}
		else if (!strncmp(pStart, "-filter:", 8)) {
			filter = pStart + 8;
		}
	}

	if (!szTargetDir[0])
		strcpy(szTargetDir, argv[1]);
/*	if (!fileExists(szTargetDir)) {
		strcpy(szTargetDir, argv[0]);
		char *p = getDirSepEnd(szTargetDir);
		*p = 0;
	}*/

	return count;
}

int main(int argc, const char **argv) {
	if (argc == 1) {
		showHelp();
		return 0;
	}

	char         szTargetDir[MAX_PATH];
	char         szCommentFile[MAX_PATH];
	std::string  sComment;
	std::string  sFilter;
	bool         bBackup;

	parsingArguments(argc, argv, szTargetDir, szCommentFile, sFilter, bBackup);

	if (!szTargetDir[0]) {
		showHelp();
		cerr << "The target directory is not found" << endl;
		return -1;
	}
#ifdef _DEBUG
//	strcpy(szTargetDir, "d:\\SlaFF\\Visual C++ 10\\addcomment\\bin\\test\\");
//	sCommentFile = "d:\\SlaFF\\Visual C++ 10\\addcomment\\bin\\comment.txt";
#endif

	char *pEnd = getDirSepEnd(szTargetDir);
	if (!pEnd) {
		size_t len = strlen(szTargetDir);
		szTargetDir[len]     = DIR_SEP_CHAR;
		szTargetDir[len + 1] = 0;
	}

	if (!readComment(szCommentFile, sComment)) {
		cerr << "Couldn't read the comment file\n" << szCommentFile << endl;
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
		addCommentToDir(szTargetDir, sComment, bBackup, szFilter);
		pFilter += len + 1;
	}
	if (pFilter && pFilter[0])
		addCommentToDir(szTargetDir, sComment, bBackup, pFilter);
}
