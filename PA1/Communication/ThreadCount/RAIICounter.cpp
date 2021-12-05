#include "RAIICounter.h"


// Default Constructor.
RAIICounter::RAIICounter(ThreadCount& instance)
	:
	m_Instance(instance)
{
	// mutex protected counter.
	m_Instance.Increment();
}

// Destructor.
RAIICounter::~RAIICounter()
{
	// mutex protected counter.
	m_Instance.Decrement();
}
