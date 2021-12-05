#include "CommandPool.h"
#include "C_LoadWaveBuffer.h"
#include "C_WaveDone.h"

// Default constructor.
CommandPool::CommandPool(const PoolType poolType)
	: m_PoolQueue(new CircularData()),
	m_PoolType(poolType)
{
	if (m_PoolType == CommandPool::PoolType::LOAD_WAVE_BUFFER)
	{
		// comamnd to add.
		C_LoadWaveBuffer* newCommand = new C_LoadWaveBuffer();

		// fill pool with load wave commands.
		while (m_PoolQueue->PushBack(newCommand))
		{
			newCommand = new C_LoadWaveBuffer();
		}

		// pool full, delete created command.
		delete newCommand;
	}
	else if (m_PoolType == CommandPool::PoolType::WAVE_DONE)
	{
		// comamnd to add.
		C_WaveDone* newCommand = new C_WaveDone();

		// fill pool with wave done commands.
		while (m_PoolQueue->PushBack(newCommand))
		{
			newCommand = new C_WaveDone();
		}

		// pool full, delete created command.
		delete newCommand;
	}
}

// Destructor.
CommandPool::~CommandPool()
{
	// command.
	Command* poppedCommand = nullptr;

	// pop and delete all commands.
	while (m_PoolQueue->PopFront(poppedCommand))
	{
		// delete command.
		delete poppedCommand;
	}
	
	// delete queue.
	delete m_PoolQueue;
}

// gets a command from pool.
Command* CommandPool::GetCommandFromPool()
{
	// command.
	Command* poppedCommand = nullptr;

	// remove command from queue.
	if (!m_PoolQueue->PopFront(poppedCommand))
	{
		// error. { increase circular data size }
		assert(false);
	}

	// return command.
	return poppedCommand;
}

// put command back to pool.
void CommandPool::AddBackToPool(Command* addedCommand)
{
	// clean command.
	addedCommand->Clean();

	// add command back to queue.
	if (!m_PoolQueue->PushBack(addedCommand))
	{
		// error. { increase circular data size }
		assert(false);
	}
}

