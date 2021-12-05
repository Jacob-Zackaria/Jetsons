#include "ThreadCount.h"

// Default constructor.
ThreadCount::ThreadCount()
	:
	m_Counter(0u), m_Mutex()
{
}

// Destructor.
ThreadCount::~ThreadCount()
{
}

// increment counter.
void ThreadCount::Increment()
{
	// lock and increment.
	std::lock_guard<std::mutex> lock(m_Mutex);
	++m_Counter;
}

// decrement counter.
void ThreadCount::Decrement()
{
	// lock and decrement.
	std::lock_guard<std::mutex> lock(m_Mutex);
	--m_Counter;

	// if counter reaches 0.
	if (m_Counter == 0u)
	{
		// set the promise to kill thread.
		m_KillPromiseFromCounter.set_value();
	}
}

// future for kill thread.
void ThreadCount::GetKillFutureFromCounter()
{
	// call function once.
	m_KillPromiseFromCounter.get_future().get();
}

