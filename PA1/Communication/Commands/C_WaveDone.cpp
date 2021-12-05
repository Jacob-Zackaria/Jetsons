#include "C_WaveDone.h"
#include "C_LoadWaveBuffer.h"

// Default Constructor.
C_WaveDone::C_WaveDone()
	:
	m_WaveIndex(0u)
{
}

// Destructor.
C_WaveDone::~C_WaveDone()
{
}

// override base execute.
void C_WaveDone::Execute(Buffer* const)
{
}

// clean command.
void C_WaveDone::Clean()
{
	m_WaveIndex = 0u;
}

