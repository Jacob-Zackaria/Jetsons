#include "C_LoadWaveBuffer.h"
#include "CoordinatorThread/CoordinatorThread.h"

// Default Constructor.
C_LoadWaveBuffer::C_LoadWaveBuffer()
	:
	m_WaveThread(nullptr)
{
}

// Destructor.
C_LoadWaveBuffer::~C_LoadWaveBuffer()
{
}

// override base execute.
void C_LoadWaveBuffer::Execute(Buffer* const sourceBuffer)
{
	// lock source buffer.
	std::lock_guard<std::mutex> lockFrontBuffer(sourceBuffer->m_BufferMutex);

	// get buffer pointer.
	const uint8_t* const bufferLocation = sourceBuffer->GetBuffer();

	// get limited size. { using this function will change buffer size and start address. }
	const uint32_t limitedSize = sourceBuffer->GetBlock(WaveBufferThread::m_WaveBufferSize);

	// fill wave thread.
	m_WaveThread->FillBuffer(bufferLocation, limitedSize);
}

// clean command.
void C_LoadWaveBuffer::Clean()
{
	// set to null.
	m_WaveThread = nullptr;
}

