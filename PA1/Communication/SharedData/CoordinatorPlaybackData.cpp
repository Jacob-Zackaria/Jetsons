#include "CoordinatorPlaybackData.h"

// Default constructor.
CoordinatorPlaybackData::CoordinatorPlaybackData()
	:
	m_CoordinatorPlaybackMutex(),
	m_Play_to_Coordinator_Queue(new CircularData()),
	m_CoordinatorCommandPool(new CommandPool(CommandPool::PoolType::LOAD_WAVE_BUFFER)),
	m_Coordinator_To_Playback_cv(),
	m_Playback_To_Coordinator_cv(),
	m_DrainStatus(DrainStatus::NOT_DRAINED),
	m_DrainMutex()
{
	// create temporary promise.
	std::promise<void> temporaryPromise;

	// get future from the promsie.
	m_FutureCoordinatorDone = temporaryPromise.get_future();

	// set promise value.
	temporaryPromise.set_value();
}

// Destructor.
CoordinatorPlaybackData::~CoordinatorPlaybackData()
{
	// command.
	Command* poppedCommand = nullptr;

	// pop and delete all commands.
	while (m_Play_to_Coordinator_Queue->PopFront(poppedCommand))
	{
		// delete command.
		delete poppedCommand;
	}

	// delete empty queue.
	delete m_Play_to_Coordinator_Queue;

	// delete command pool.
	delete m_CoordinatorCommandPool;
}

// set drain status.
void CoordinatorPlaybackData::SetDrainedStatus(const DrainStatus newStatus)
{
	// lock.
	std::lock_guard<std::mutex> lockDrainStatus(m_DrainMutex);

	// set status.
	m_DrainStatus = newStatus;
}

// drain status { drained => true }
const bool CoordinatorPlaybackData::GetDrainedStatus()
{
	// lock.
	std::lock_guard<std::mutex> lockDrainStatus(m_DrainMutex);

	// return status
	return (bool)m_DrainStatus;
}


