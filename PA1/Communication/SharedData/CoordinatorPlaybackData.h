#ifndef COORDINATOR_PLAYBACK_DATA_H
#define COORDINATOR_PLAYBACK_DATA_H

#include "Commands/CommandPool.h"

// structure which is shared across coordinator and playback threads.
struct CoordinatorPlaybackData
{
public:

	//----------------Enum----------------//

	enum class DrainStatus
	{
		NOT_DRAINED,
		DRAINED
	};

	//------------------------------------//

	//-----Constructors and Destructor-----//

	CoordinatorPlaybackData();
	CoordinatorPlaybackData(const CoordinatorPlaybackData&) = delete;
	CoordinatorPlaybackData& operator = (const CoordinatorPlaybackData&) = delete;
	CoordinatorPlaybackData(CoordinatorPlaybackData&&) = default;
	CoordinatorPlaybackData& operator = (CoordinatorPlaybackData&&) = default;
	~CoordinatorPlaybackData();

	//------------------------------------//

	//----------Public Methods-----------//

	// drain status { drained => true }
	const bool GetDrainedStatus();

	// set drain status.
	void SetDrainedStatus(const DrainStatus newStatus);

	//------------------------------------//

	//------------Public Data-------------//

	// mutex shared between coordinator and playback thread.
	std::mutex m_CoordinatorPlaybackMutex;

	// queue from playback to coordinator.
	CircularData* m_Play_to_Coordinator_Queue;

	// command pool.
	CommandPool* m_CoordinatorCommandPool;

	// condition variable.
	std::condition_variable m_Coordinator_To_Playback_cv;

	// condition variable.
	std::condition_variable m_Playback_To_Coordinator_cv;

	// future for coordinator done.
	std::future<void> m_FutureCoordinatorDone;

	//------------------------------------//

private:

	//-------------- Data-----------------//

	// used in coordinator-playback wave buffer fill.
	CoordinatorPlaybackData::DrainStatus m_DrainStatus;

	// mutex for drain status.
	std::mutex m_DrainMutex;

	//------------------------------------//

};


#endif COORDINATOR_PLAYBACK_DATA_H