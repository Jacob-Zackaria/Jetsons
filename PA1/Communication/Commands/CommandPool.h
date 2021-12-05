#ifndef COMMAND_POOL_H
#define COMMAND_POOL_H

#include "CircularQueue/CircularData.h"

// class to store a pool of commands.
class CommandPool
{
public:

	//---------------Enum------------------//

	enum class PoolType
	{
		LOAD_WAVE_BUFFER,
		WAVE_DONE
	};

	//------------------------------------//

	//-----Constructors and Destructor-----//

	CommandPool(const PoolType poolType);
	CommandPool(const CommandPool&) = delete;
	CommandPool& operator = (const CommandPool&) = delete;
	CommandPool(CommandPool&&) = default;
	CommandPool& operator = (CommandPool&&) = default;
	~CommandPool();

	//------------------------------------//

	//---------Public methods-------------//

	// gets a command from pool.
	Command* GetCommandFromPool();

	// put command back to pool. { cleans data }
	void AddBackToPool(Command* addedCommand);

	//------------------------------------//

private:

	//----------------Data----------------//

	// circular queue as pool. 
	CircularData* m_PoolQueue;

	// type of pool.
	PoolType m_PoolType;

	//------------------------------------//
};



#endif COMMAND_POOL_H