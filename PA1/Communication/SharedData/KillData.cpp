#include "KillData.h"

// Default constructor.
KillData::KillData()
	:
	m_CounterForKill()
{
	// create temporary promise.
	std::promise<void> temporaryPromise;

	// get future from the promsie.
	m_FutureFromShared = temporaryPromise.get_future();

	// set promise value.
	temporaryPromise.set_value();
}

// Destructor.
KillData::~KillData()
{
}

// kill all threads.
void KillData::NotifyThreadsToExit()
{
	// get the future value { makes all shared future invalid }
	m_FutureFromShared.get();
}

// future for threads.
std::future<void>& KillData::GetFutureFromShared()
{
	// share future with threads calling this function.
	return std::ref(m_FutureFromShared);
}

// promise for kill thread.
void KillData::InitiateKill()
{
	m_PromiseToKill.set_value();
}

// future for kill thread.
void KillData::WaitForKill()
{
	m_PromiseToKill.get_future().get();
}

// wait for thread counter's future.get()
void KillData::WaitTillKillDone()
{
	// call get.
	m_CounterForKill.GetKillFutureFromCounter();
}

// get instance of thread counter.
ThreadCount& KillData::GetThreadCounter()
{
	// share this counter across file-coordinator-playback threads.
	return std::ref(m_CounterForKill);
}
