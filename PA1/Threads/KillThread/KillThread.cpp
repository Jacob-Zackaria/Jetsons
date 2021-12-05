#include "KillThread.h"

// Specialized Constructor.
KillThread::KillThread(const char* const threadName)
	: BannerBase(threadName)
{
}

// Destructor.
KillThread::~KillThread()
{
}

// function call operator.
void KillThread::operator()(KillData& killData)
{
	START_BANNER

	// wait for kill from playback.
	killData.WaitForKill();

	// Initiate kill.
	killData.NotifyThreadsToExit();

	// wait until all threads are killed. { future.get() called inside function }
	killData.WaitTillKillDone();
}
