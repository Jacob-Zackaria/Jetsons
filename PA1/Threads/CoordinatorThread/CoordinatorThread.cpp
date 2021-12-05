#include "CoordinatorThread.h"
#include "ThreadCount/RAIICounter.h"
#include "FileThread/FileThread.h"
#include "Commands/C_LoadWaveBuffer.h"

using namespace std::chrono_literals;

// Specialized Constructor.
CoordinatorThread::CoordinatorThread(const char* const threadName, FileCoordinatorData& fileCoordinatorData, CoordinatorPlaybackData& coordinatorPlaybackData)
	: BannerBase(threadName), m_CoordinatorThread(),
	m_FrontBuffer(new Buffer(m_CoordinatorBufferSize)), m_BackBuffer(new Buffer(m_CoordinatorBufferSize)),
	m_CoordinatorState(CoordinatorThread::CoordinatorState::FILL),
	m_FileCoordinatorData(fileCoordinatorData), m_CoordinatorPlaybackData(coordinatorPlaybackData)
{
}

// Destructor.
CoordinatorThread::~CoordinatorThread()
{
	// delete buffers.
	delete m_FrontBuffer;
	delete m_BackBuffer;

	// check if joinable.
	if (m_CoordinatorThread.joinable())
	{
		// join thread before destruction.
		m_CoordinatorThread.join();
	}
}

// launch object as thread.
void CoordinatorThread::Launch(KillData& killData)
{
	// check if not joinable.
	if (!m_CoordinatorThread.joinable())
	{
		// spawn a new thread with 'this' object reference passed as callable object.
		m_CoordinatorThread = std::thread(std::ref(*this), std::ref(killData));
	}
	else
	{
		// error. don't launch twice on same object.
		assert(false);
	}
}

// function call operator.
void CoordinatorThread::operator()(KillData& killData)
{
	START_BANNER

	//--------------------------------------- Thread Start ---------------------------------------------------//

	// Mutex protected thread counter { Increments during construction, decrements during destruction }
	const RAIICounter c_Coordinator(killData.GetThreadCounter());

	// get shared future from kill data.
	std::future<void>& sharedFuture = killData.GetFutureFromShared();

	// Commands from playback queue.
	Command* inputCommand = nullptr;

	// lamda { if drainstatus is not_drained => true }
	const auto drainedStatus = [&]() -> const bool { return !(m_CoordinatorPlaybackData.GetDrainedStatus()); };

	//>>>> preload two wave files.
	for (uint8_t i = 0; i < 2; ++i)
	{
		// swap buffers.
		Debug::out("Swap Buffers (front)<->(back)\n");
		std::swap(m_FrontBuffer, m_BackBuffer);

		//Debug::out("Filling Back Buffer\n");

		// lock back buffer.
		std::unique_lock<std::mutex> lockBackBuffer(m_BackBuffer->m_BufferMutex);

		// wait till a notify is recieved and file buffer is full.
		m_FileCoordinatorData.m_File_to_Coordinator_cv.wait(lockBackBuffer, std::move(&FileThread::GetFileBufferStatus));

		// transfer data from file buffer to back buffer.
		FileThread::TransferDataFromFile(m_BackBuffer, m_CoordinatorBufferSize);

		// notify file thread.
		m_FileCoordinatorData.m_Coordinator_to_File_cv.notify_one();

		//Debug::out("Filled Back Buffer\n");
	}
	
	// change state to drain.
	m_CoordinatorState = CoordinatorThread::CoordinatorState::DRAIN;

	//--------------------------------------------------------------------------------------------------------//

	//---------------------------------------- Thread loop ---------------------------------------------------//

	// (if future is still valid) => loop.
	while (sharedFuture.valid())
	{
		// check if all buffers are done.
		if (BuffersDone())
		{
			// exit thread. { all buffers done }
			break;
		}

		// (if coordinator is in FILL state or WAIT state) and (if file buffer is full)
		if ((m_CoordinatorState == CoordinatorThread::CoordinatorState::FILL) && FileThread::GetFileBufferStatus())
		{
			// transfer data from file buffer to back buffer.
			FileThread::TransferDataFromFile(m_BackBuffer, m_CoordinatorBufferSize);

			// notify file thread.
			m_FileCoordinatorData.m_Coordinator_to_File_cv.notify_one();

			// change state to drain.
			m_CoordinatorState = CoordinatorThread::CoordinatorState::DRAIN;	
		}
		
		// if front buffer is not empty => drain.
		if (m_FrontBuffer->GetStatus() != Buffer::Status::EMPTY)
		{
			// lock coordinator thread.
			std::unique_lock<std::mutex> lockCoordinator(m_CoordinatorPlaybackData.m_CoordinatorPlaybackMutex);

			// wait for playback notify.
			m_CoordinatorPlaybackData.m_Playback_To_Coordinator_cv.wait(lockCoordinator, std::move(drainedStatus));

			// if there are commands from playback.
			if (m_CoordinatorPlaybackData.m_Play_to_Coordinator_Queue->PopFront(inputCommand))
			{
				// make sure command is available.
				assert(inputCommand);

				// execute the command. { pass in source buffer pointer }
				inputCommand->Execute(m_FrontBuffer);

				// set drained status.
				m_CoordinatorPlaybackData.SetDrainedStatus(CoordinatorPlaybackData::DrainStatus::DRAINED);

				// notify playback thread.
				m_CoordinatorPlaybackData.m_Coordinator_To_Playback_cv.notify_one();

				// recycle command. { cleans and adds back to pool }
				m_CoordinatorPlaybackData.m_CoordinatorCommandPool->AddBackToPool(inputCommand);
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------//

	//------------------------------------------ Thrad End ---------------------------------------------------//

	// notify playback thread, that coordinator is done.
	m_CoordinatorPlaybackData.m_FutureCoordinatorDone.get();

	Debug::out("DONE! All buffers loaded.\n");

	//--------------------------------------------------------------------------------------------------------//
}

// switch buffers if needed.
const bool CoordinatorThread::BuffersDone()
{
	// check if (front-buffer is empty) { mutex protected access. }
	if (m_FrontBuffer->GetStatus() == Buffer::Status::EMPTY)
	{
		// check if (file thread is running)
		if (m_FileCoordinatorData.m_FutureFileDone.valid())
		{
			// if back-buffer is not empty. { mutex protected access. }
			if (m_BackBuffer->GetStatus() != Buffer::Status::EMPTY)
			{
				// swap buffers.
				Debug::out("Swap Buffers (front)<->(back)\n");
				std::swap(m_FrontBuffer, m_BackBuffer);
			}

			// change state to fill.
			m_CoordinatorState = CoordinatorThread::CoordinatorState::FILL;	
		}

		// else if (back buffer is also empty) { mutex protected access. }
		else if (m_BackBuffer->GetStatus() == Buffer::Status::EMPTY)
		{
			// exit coordinator thread.
			return true;
		}

		// else data available on back-buffer.
		else
		{
			// swap buffers.
			Debug::out("Swap Buffers (front)<->(back)\n");
			std::swap(m_FrontBuffer, m_BackBuffer);
		}
	}

	// continue.
	return false;
}
