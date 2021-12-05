#ifndef PLAYBACK_WAVE_DATA_H
#define PLAYBACK_WAVE_DATA_H

#include "Commands/CommandPool.h"
#include "ThreadCount/ThreadCount.h"

// structure which is shared across playback and wave threads.
struct PlaybackWaveData
{
	//-----Constructors and Destructor-----//

	PlaybackWaveData();
	PlaybackWaveData(const PlaybackWaveData&) = delete;
	PlaybackWaveData& operator = (const PlaybackWaveData&) = delete;
	PlaybackWaveData(PlaybackWaveData&&) = default;
	PlaybackWaveData& operator = (PlaybackWaveData&&) = default;
	~PlaybackWaveData();

	//------------------------------------//

	//------------Public Methods----------//

	// future for wave threads.
	std::future<void>& GetFutureFromPlayback();

	// kill all wave threads.
	void NotifyThreadsToExit();

	// wait for thread counter's future.get()
	void WaitTillKillDone();

	// get instance of thread counter.
	ThreadCount& GetThreadCounter();

	//------------------------------------//

	//------------Public Data-------------//

	// device handle for wave playback.
	HWAVEOUT m_DeviceHandle;

	// queue from wave callback to playback.
	CircularData* m_Wave_to_Playback_Queue;

	// command pool.
	CommandPool* m_PlaybackCommandPool;

	// condition variable.
	std::condition_variable m_Wave_To_Playback_cv;

	//------------------------------------//

private:

	//-----------------Data---------------//

	// future for wave threads.
	std::future<void> m_FutureFromPlayback;

	// thread count instance for file-coordinator-playback threads.
	ThreadCount m_CounterForWave;

	//------------------------------------//

};


#endif PLAYBACK_WAVE_DATA_H