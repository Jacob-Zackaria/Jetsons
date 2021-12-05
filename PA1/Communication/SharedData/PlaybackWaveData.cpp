#include "PlaybackWaveData.h"

// Default constructor.
PlaybackWaveData::PlaybackWaveData()
	:
	m_DeviceHandle(nullptr),
	m_Wave_to_Playback_Queue(new CircularData()),
	m_PlaybackCommandPool(new CommandPool(CommandPool::PoolType::WAVE_DONE)),
	m_Wave_To_Playback_cv(),
	m_CounterForWave()
{
	// create temporary promise.
	std::promise<void> temporaryPromise;

	// get future from the promsie.
	m_FutureFromPlayback = temporaryPromise.get_future();

	// set promise value.
	temporaryPromise.set_value();
}

// Destructor.
PlaybackWaveData::~PlaybackWaveData()
{
	// command.
	Command* poppedCommand = nullptr;

	// pop and delete all commands.
	while (m_Wave_to_Playback_Queue->PopFront(poppedCommand))
	{
		// delete command.
		delete poppedCommand;
	}

	// delete queue.
	delete m_Wave_to_Playback_Queue;

	// delete command pool.
	delete m_PlaybackCommandPool;
}

// future for wave threads.
std::future<void>& PlaybackWaveData::GetFutureFromPlayback()
{
	// share future with threads calling this function.
	return std::ref(m_FutureFromPlayback);
}

// kill all wave threads.
void PlaybackWaveData::NotifyThreadsToExit()
{
	// get the future value { makes all shared future invalid }
	m_FutureFromPlayback.get();
}

// wait for thread counter's future.get()
void PlaybackWaveData::WaitTillKillDone()
{
	// call get.
	m_CounterForWave.GetKillFutureFromCounter();
}

// get instance of thread counter.
ThreadCount& PlaybackWaveData::GetThreadCounter()
{
	// share this counter across all wave threads.
	return std::ref(m_CounterForWave);
}

