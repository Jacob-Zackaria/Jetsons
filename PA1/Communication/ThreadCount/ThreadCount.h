#ifndef THREAD_COUNT_H
#define THREAD_COUNT_H

// class to count threads
class ThreadCount
{
public:

	//-----Constructors and Destructor-----//

	ThreadCount();
	ThreadCount(const ThreadCount&) = delete;
	ThreadCount& operator = (const ThreadCount&) = delete;
	ThreadCount(ThreadCount&&) = default;
	ThreadCount& operator = (ThreadCount&&) = default;
	~ThreadCount();

	//------------------------------------//

	//---------Public methods-------------//

	// increment counter.
	void Increment();

	// decrement counter.
	void Decrement();

	// future for kill thread.
	void GetKillFutureFromCounter();

	//------------------------------------//

private:

	//----------------Data----------------//

	// 8 bit counter.
	uint8_t m_Counter;

	// mutex for counter.
	std::mutex m_Mutex;

	// promise for kill thread.
	std::promise<void> m_KillPromiseFromCounter;

	//------------------------------------//

};


#endif THREAD_COUNT_H