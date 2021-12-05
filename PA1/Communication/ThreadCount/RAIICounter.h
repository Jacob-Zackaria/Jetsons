#ifndef RAII_COUNTER_H
#define RAII_COUNTER_H

#include "ThreadCount.h"

// RAII structure used for thread counting.
struct RAIICounter
{
	//-----Constructors and Destructor-----//

	RAIICounter(ThreadCount& instance);
	RAIICounter(const RAIICounter&) = delete;
	RAIICounter& operator = (const RAIICounter&) = delete;
	RAIICounter(RAIICounter&&) = default;
	RAIICounter& operator = (RAIICounter&&) = default;
	~RAIICounter();

	//------------------------------------//

private:

	//----------------Data---------------//

	ThreadCount& m_Instance;

	//------------------------------------//

};

#endif RAII_COUNTER_H