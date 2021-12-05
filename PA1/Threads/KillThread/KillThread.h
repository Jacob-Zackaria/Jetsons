#ifndef KILL_THREAD_H
#define KILL_THREAD_H

#include "SharedData/KillData.h"

// used for thread functions.
using namespace ThreadFramework;

// struct for kill thread.
struct KillThread : public BannerBase
{
	//-----Constructors and Destructor-----//

	KillThread(const char* const threadName);
	KillThread(const KillThread&) = delete;
	KillThread& operator = (const KillThread&) = delete;
	KillThread(KillThread&&) = default;
	KillThread& operator = (KillThread&&) = default;
	~KillThread();

	//------------------------------------//

	//---------Public methods-------------//

	// function call operator.
	void operator ()(KillData& killData);

	//------------------------------------//

};



#endif KILL_THREAD_H