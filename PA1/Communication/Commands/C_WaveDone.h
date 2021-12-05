#ifndef C_WAVE_DONE_H
#define C_WAVE_DONE_H

#include "Command.h"

// class for sending wave done command.
class C_WaveDone : public Command
{
public:

	//-----Constructors and Destructor-----//

	C_WaveDone();
	C_WaveDone(const C_WaveDone&) = delete;
	C_WaveDone& operator = (const C_WaveDone&) = delete;
	C_WaveDone(C_WaveDone&&) = default;
	C_WaveDone& operator = (C_WaveDone&&) = default;
	virtual ~C_WaveDone();

	//------------------------------------//

	//-----------Public methods-----------//

	// override base execute.
	virtual void Execute(Buffer* const sourceBuffer = nullptr) override;

	// clean command.
	virtual void Clean() override;

	//------------------------------------//
	
	//----------Public Data---------------//

	uint8_t m_WaveIndex;

	//------------------------------------//

};


#endif C_WAVE_DONE_H