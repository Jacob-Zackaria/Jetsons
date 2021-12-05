#ifndef COORDINATOR_THREAD_H
#define COORDINATOR_THREAD_H

#include "SharedData/KillData.h"
#include "SharedData/FileCoordinatorData.h"
#include "SharedData/CoordinatorPlaybackData.h"
#include "Buffer.h"

// used for thread functions.
using namespace ThreadFramework;

// Class for coordinator thread.
class CoordinatorThread : public BannerBase
{
public:

	//--------Static constant data----------//

	// coordiantor buffer size.
	static const uint32_t m_CoordinatorBufferSize = 512u * 1024u;

	//-------------------------------------//

	//-----------------Enum----------------//

	enum class CoordinatorState
	{
		FILL,
		DRAIN
	};

	//-------------------------------------//

	//-----Constructors and Destructor-----//

	CoordinatorThread(const char* const threadName, FileCoordinatorData& fileCoordinatorData, CoordinatorPlaybackData& coordinatorPlaybackData);
	CoordinatorThread(const CoordinatorThread&) = delete;
	CoordinatorThread& operator = (const CoordinatorThread&) = delete;
	CoordinatorThread(CoordinatorThread&&) = default;
	CoordinatorThread& operator = (CoordinatorThread&&) = default;
	~CoordinatorThread();

	//------------------------------------//

	//---------Public methods-------------//

	// launch object as thread.
	void Launch(KillData& killData);

	// function call operator.
	void operator ()(KillData& killData);

	//------------------------------------//

private:

	//---------Private methods------------//

	// switch buffers if needed, else exit coordinator thread.
	const bool BuffersDone();

	//------------------------------------//

	//----------------Data----------------//

	// thread.
	std::thread	m_CoordinatorThread;

	// front buffer for coordinator thread.
	Buffer* m_FrontBuffer;

	// back buffer for coordinator thread.
	Buffer* m_BackBuffer;

	// coordinator state.
	CoordinatorState m_CoordinatorState;

	// file-coordinator shared data.
	FileCoordinatorData& m_FileCoordinatorData;

	// coordinator-playback shared data.
	CoordinatorPlaybackData& m_CoordinatorPlaybackData;

	//------------------------------------//
};


#endif COORDINATOR_THREAD_H