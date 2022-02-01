#include "FileThread.h"
#include "ThreadCount/RAIICounter.h"
#include "FileSlow.h"

using namespace std::chrono_literals;

// static declaration.
FileThread* FileThread::staticInstance = nullptr;

// Specialized Constructor.
FileThread::FileThread(const char* const threadName, FileCoordinatorData& fileCoordinatorData)
	: BannerBase(threadName), m_FileThread(),
	m_FileBuffer(new Buffer(m_FileBufferSize)),
	m_FileCoordinatorData(fileCoordinatorData)
{
	// set singleton instance.
	FileThread::staticInstance = this;
}

// Destructor.
FileThread::~FileThread()
{
	// delete buffer storage.
	delete m_FileBuffer;

	// check if joinable.
	if (m_FileThread.joinable())
	{
		// join thread before destruction.
		m_FileThread.join();
	}
}

// launch object as thread.
void FileThread::Launch(KillData& killData)
{
	// check if not joinable.
	if (!m_FileThread.joinable())
	{
		// spawn a new thread with 'this' object reference passed as callable object.
		m_FileThread = std::thread(std::ref(*this), std::ref(killData));
	}
	else
	{
		// error. don't launch twice on same object.
		assert(false);
	}
}

// function call operator.
void FileThread::operator()(KillData& killData)
{
	START_BANNER

	//--------------------------------------- Thread Start ---------------------------------------------------//

	// Mutex protected thread counter { Increments during construction, decrements during destruction }
	const RAIICounter c_File(killData.GetThreadCounter());
	
	// get shared future from kill data.
	std::future<void>& sharedFuture = killData.GetFutureFromShared();

	// wave count.
	const uint8_t totalNumberOfWaves = 22u; uint8_t currentWave = 0u;

	// wave name.
	char waveName[16u] = {};

	// lamda { if file buffer is empty => true }
	const auto fileBufferStatus = [&]() { return !((bool)m_FileBuffer->GetStatus()); };

	//>>>> preload two wave files.
	for (uint8_t i = 0; i < 2; ++i)
	{
		// get wave name.
		sprintf_s(waveName, 16u, "wave_%d.wav", currentWave);

		Debug::out("Loading %s\n", waveName);

		// load wave.
		privLoadFile(waveName);

		// notify coordinator thread.
		m_FileCoordinatorData.m_File_to_Coordinator_cv.notify_one();

		Debug::out("%s Loaded!\n", waveName);

		// lock file buffer.
		std::unique_lock<std::mutex> lockFileBuffer(m_FileBuffer->m_BufferMutex);

		// wait till a notify is received and file buffer is empty.
		m_FileCoordinatorData.m_Coordinator_to_File_cv.wait(lockFileBuffer, std::move(fileBufferStatus));

		// increment wave count.
		++currentWave;
	}

	//--------------------------------------------------------------------------------------------------------//

	//---------------------------------------- Thread loop ---------------------------------------------------//

	do
	{
		// get wave name.
		sprintf_s(waveName, 16u, "wave_%d.wav", currentWave);

		Debug::out("Loading %s\n", waveName);

		// load wave.
		privLoadFile(waveName);

		Debug::out("%s Loaded!\n", waveName);

		// lock file buffer.
		std::unique_lock<std::mutex> lockFileBuffer(m_FileBuffer->m_BufferMutex);

		// wait till a notify is recieved and file buffer is empty.
		m_FileCoordinatorData.m_Coordinator_to_File_cv.wait(lockFileBuffer, std::move(fileBufferStatus));

		// if all waves done.
		if (currentWave == totalNumberOfWaves)
		{
			// exit thread.
			break;
		}

		// increment wave count.
		++currentWave;
	
		Debug::out("Sleep (200ms)\n");

		// put the file thread to sleep.
		std::this_thread::sleep_for(200ms);

	// (if future is still valid) => loop.
	} while (sharedFuture.valid());

	//--------------------------------------------------------------------------------------------------------//

	//------------------------------------------ Thread End ---------------------------------------------------//

	// notify coordinator thread, that file is done.
	m_FileCoordinatorData.m_FutureFileDone.get();

	Debug::out("DONE! All waves loaded.\n");

	//--------------------------------------------------------------------------------------------------------//
}

// load given file to buffer.
void FileThread::privLoadFile(const char* const fileName)
{
	// file handle.
	FileSlow::Handle fileHandle = nullptr;

	// file error status.
	FileSlow::Error fileError = FileSlow::Error::UNDEFINED;

	// file size.
	DWORD fileSize = 0ul;

	// open file for read.
	fileError = FileSlow::Open(fileHandle, fileName, FileSlow::Mode::READ);
	assert(fileError == FileSlow::Error::SUCCESS);

	// seek to the end for size.
	fileError = FileSlow::Seek(fileHandle, FileSlow::Location::END, 0);
	assert(fileError == FileSlow::Error::SUCCESS);

	// get file size.
	fileError = FileSlow::Tell(fileHandle, fileSize);
	assert(fileError == FileSlow::Error::SUCCESS);

	// seek to beginning for read.
	fileError = FileSlow::Seek(fileHandle, FileSlow::Location::BEGIN, 0);
	assert(fileError == FileSlow::Error::SUCCESS);

	// lock file buffer.
	std::lock_guard<std::mutex> lockFileBuffer(m_FileBuffer->m_BufferMutex);

	// read file to buffer.
	fileError = FileSlow::Read(fileHandle, m_FileBuffer->GetBuffer(), fileSize);
	assert(fileError == FileSlow::Error::SUCCESS);

	// set file buffer used size.
	m_FileBuffer->SetUsedSize(fileSize);

	// set file buffer as full. { mutex protected access. }
	m_FileBuffer->SetStatus(Buffer::Status::FULL);

	// close file.
	fileError = FileSlow::Close(fileHandle);
	assert(fileError == FileSlow::Error::SUCCESS);
}

// transfer data from file buffer to coordinator back-buffer.
void FileThread::TransferDataFromFile(Buffer* const coordinatorBackBufferPointer, const uint32_t bufferSize)
{
	// lock file buffer.
	std::lock_guard<std::mutex> lock(staticInstance->m_FileBuffer->m_BufferMutex);

	// get buffer pointer.
	const uint8_t* bufferLocation = staticInstance->m_FileBuffer->GetBuffer();

	// get limited size. { using this function will change buffer size and start address. }
	const uint32_t limitedSize = staticInstance->m_FileBuffer->GetBlock(bufferSize);

	// fill coordinator buffer.
	coordinatorBackBufferPointer->FillBuffer(bufferLocation, limitedSize);

	Debug::out("File --> Coordinator | Transfer Size:[%u]\n", limitedSize);
}

// get file buffer status.
const bool FileThread::GetFileBufferStatus()
{
	// mutex protected access.
	return (bool)staticInstance->m_FileBuffer->GetStatus();
}
