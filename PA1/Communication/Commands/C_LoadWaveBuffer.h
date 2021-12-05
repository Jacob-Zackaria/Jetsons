#ifndef C_LOAD_WAVE_BUFFER_H
#define C_LOAD_WAVE_BUFFER_H

#include "Command.h"
#include "WaveBufferThread/WaveBufferThread.h"

// class for loading wave buffer command.
class C_LoadWaveBuffer : public Command
{
public:

	//-----Constructors and Destructor-----//

	C_LoadWaveBuffer();
	C_LoadWaveBuffer(const C_LoadWaveBuffer&) = delete;
	C_LoadWaveBuffer& operator = (const C_LoadWaveBuffer&) = delete;
	C_LoadWaveBuffer(C_LoadWaveBuffer&&) = default;
	C_LoadWaveBuffer& operator = (C_LoadWaveBuffer&&) = default;
	virtual ~C_LoadWaveBuffer();

	//------------------------------------//

	//-----------Public methods-----------//

	// override base execute.
	virtual void Execute(Buffer* const sourceBuffer = nullptr) override;

	// clean command.
	virtual void Clean() override;

	//------------------------------------//

	//---------Public Data----------------//

	// destination buffer.
	WaveBufferThread* m_WaveThread;

	//------------------------------------//
};


#endif C_LOAD_WAVE_BUFFER_H