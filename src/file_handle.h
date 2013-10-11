/*
file_hundle.h
*/
#ifdef _WIN32
	#include <Windows.h>
#else
	#include <sys/types.h>	
	#include <sys/dirent.h>
#endif


class FileHandler {
public:
	FileHandler();
	~FileHandler();

	bool        getFirstFile(char *szDirName);
	bool        getNextFile();
	void        close();
	const char* getFileName() const;

	bool isDirectory() const;
	bool isWriteableFile() const;

	operator const char*() const;

private:
#ifdef _WIN32
	WIN32_FIND_DATA  m_wfd;
	HANDLE           m_hFileHandler;
#else
	DIR             *m_dir;
	struct dirent   *m_entryDir;
	struct dirent  **m_pEntryDir;
#endif

};