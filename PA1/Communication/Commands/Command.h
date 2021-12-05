//----------------------------------------------------------------------------
// Copyright 2021, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------

#ifndef COMMAND_H
#define COMMAND_H

// forward declaration.
class Buffer;

// Commands 
class Command
{
public:
	// Contract for the command
	virtual void Execute(Buffer* const sourceBuffer = nullptr) = 0;

	// clean commands.
	virtual void Clean() = 0;

	// virtual destructor.
	virtual ~Command()
	{
	}
};

#endif

// --- End of File ---

