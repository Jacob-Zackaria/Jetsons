#ifndef FILE_THREAD_H
#define FILE_THREAD_H

#include "SharedData/KillData.h"
#include "SharedData/FileCoordinatorData.h"
#include "Buffer.h"

// used for thread functions.
using namespace ThreadFramework;

// Class for file thread.
class FileThread : public BannerBase
{
public:

	//--------Static constant data---------//

	// file buffer size.
	static const uint32_t m_FileBufferSize = 512u * 1024u;

	//-------------------------------------//

	//-----Constructors and Destructor-----//

	FileThread(const char* const threadName, FileCoordinatorData& fileCoordinatorData);
	FileThread(const FileThread&) = delete;
	FileThread& operator = (const FileThread&) = delete;
	FileThread(FileThread&&) = default;
	FileThread& operator = (FileThread&&) = default;
	~FileThread();

	//------------------------------------//

	//---------Public methods-------------//

	// launch object as thread.
	void Launch(KillData& killData);

	// function call operator.
	void operator ()(KillData& killData);

	// transfer data from file buffer to coordinator back-buffer.
	static void TransferDataFromFile(Buffer* const coordinatorBackBufferPointer, const uint32_t bufferSize);

	// get file buffer status.
	static const bool GetFileBufferStatus();

	//------------------------------------//

private:

	//---------Private methods------------//

	// load given file to buffer.
	void privLoadFile(const char* const fileName);

	//------------------------------------//

	//----------------Data----------------//

	// singleton instance.
	static FileThread* staticInstance;

	// thread.
	std::thread	m_FileThread;

	// buffer for file thread.
	Buffer* m_FileBuffer;

	// file-coordinator shared data.
	FileCoordinatorData& m_FileCoordinatorData;

	//------------------------------------//
};




#endif FILE_THREAD_H