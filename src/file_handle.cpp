/*

*/
#include "file_handle.h"

FileHandler::FileHandler() {
	m_hFileHandler = NULL;
	memset(&m_wfd, 0, sizeof(m_wfd));
}

FileHandler::~FileHandler() {
	close();
}

bool FileHandler::getFirstFile(char *szDirName) {
#ifdef _WIN32
	if (HANDLE res = FindFirstFile(szDirName, &m_wfd)) {
		m_hFileHandler = res;
		return true;
	}
#else
	m_dir = opendir(szDirName);
	if (dir) {
		return true;
	}
#endif

	return false;
}

bool FileHandler::getNextFile() {
#ifdef _WIN32
	return FindNextFile(m_hFileHandler, &m_wfd) > 0;
#else
	m_entryDir = readdir(m_dir)
	return m_entryDir != NULL;
#endif
}

void FileHandler::close() {
#ifdef _WIN32
	if (m_hFileHandler) {
		FindClose(m_hFileHandler);
		m_hFileHandler = NULL;
	}
#else
	if (m_dir) {
		closedir(m_dir);
		m_dir = NULL;
	}
#endif
}

bool FileHandler::isDirectory() const {
#ifdef _WIN32
	return (m_wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0;
#else
	return (m_entryDir->d_type == DT_DIR);
#endif
}

bool FileHandler::isWriteableFile() const {
#ifdef _WIN32
	return (!(m_wfd.dwFileAttributes & (
		FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_OFFLINE |
		FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM)));
#else
	return (m_entryDir->d_type & DT_REG);
#endif
}

const char* FileHandler::getFileName() const {
#ifdef _WIN32
	return m_wfd.cFileName;
#else
	return m_entryDir->d_name;
#endif
}

FileHandler::operator const char*() const {
	return getFileName();
}