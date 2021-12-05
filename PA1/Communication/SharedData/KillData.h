#ifndef KILL_DATA_H
#define KILL_DATA_H

#include "ThreadCount/ThreadCount.h"

// class to kill all threads.
class KillData
{
public:

	//-----Constructors and Destructor-----//

	KillData();
	KillData(const KillData&) = delete;
	KillData& operator = (const KillData&) = delete;
	KillData(KillData&&) = default;
	KillData& operator = (KillData&&) = default;
	~KillData();

	//------------------------------------//

	//---------Public methods-------------//

	// kill all threads. { makes future invalid }
	void NotifyThreadsToExit();

	// future for threads { shared across threads }
	std::future<void>& GetFutureFromShared();

	// set kill promise. { used by playback }
	void InitiateKill();

	// future for kill thread. { used by kill thread }
	void WaitForKill();

	// wait for thread counter's future.get()
	void WaitTillKillDone();

	// get instance of thread counter.
	ThreadCount& GetThreadCounter();

	//------------------------------------//

	
private:

	//----------------Data----------------//

	// future for threads.
	std::future<void> m_FutureFromShared;

	// promise for kill thread.
	std::promise<void> m_PromiseToKill;

	// thread count instance for file-coordinator-playback threads.
	ThreadCount m_CounterForKill;

	//------------------------------------//
};


#endif KILL_DATA_H