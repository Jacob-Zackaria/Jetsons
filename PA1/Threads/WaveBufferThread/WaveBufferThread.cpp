#include "WaveBufferThread.h"
#include "ThreadCount/RAIICounter.h"

// Specialized Constructor.
WaveBufferThread::WaveBufferThread(const char* const threadName, PlaybackWaveData& playbackWaveData, const uint8_t newIndex)
	: BannerBase(threadName), m_Wave_cv(), m_WaveBufferThread(),
	m_WaveBuffer(new Buffer(m_WaveBufferSize)), m_WaveHeader(new WAVEHDR()),
	m_PlaybackWaveData(playbackWaveData),
	m_WaveIndex(newIndex)
{
	// clear wave header.
	memset(m_WaveHeader, 0, sizeof(WAVEHDR));

	// set buffer size.
	m_WaveHeader->dwBufferLength = (DWORD)m_WaveBufferSize;

	// set pointer to data.
	m_WaveHeader->lpData = (LPSTR)m_WaveBuffer->GetBuffer();

	// set user instance.
	m_WaveHeader->dwUser = (DWORD_PTR)&m_WaveIndex;

	// prepare wave header.
	MMRESULT result;
	result = waveOutPrepareHeader(m_PlaybackWaveData.m_DeviceHandle, m_WaveHeader, sizeof(WAVEHDR));

	// assert no error.
	assert(result == MMSYSERR_NOERROR);

}

// Destructor.
WaveBufferThread::~WaveBufferThread()
{
	// unprepare the wave header.
	MMRESULT result;
	result = waveOutUnprepareHeader(m_PlaybackWaveData.m_DeviceHandle, m_WaveHeader, sizeof(WAVEHDR));

	// assert no error.
	assert(result == MMSYSERR_NOERROR);

	// delete buffer.
	delete m_WaveBuffer;

	// delete wave header.
	delete m_WaveHeader;

	// check if joinable.
	if (m_WaveBufferThread.joinable())
	{
		// join thread before destruction.
		m_WaveBufferThread.join();
	}
}

// launch object as thread.
void WaveBufferThread::Launch()
{
	// check if not joinable.
	if (!m_WaveBufferThread.joinable())
	{
		// spawn a new thread with 'this' object reference passed as callable object.
		m_WaveBufferThread = std::thread(std::ref(*this));
	}
	else
	{
		// error. don't launch twice on same object.
		assert(false);
	}
}

// function call operator.
void WaveBufferThread::operator()()
{
	START_BANNER

	//--------------------------------------- Thread Start ---------------------------------------------------//

	// Mutex protected thread counter { Increments during construction, decrements during destruction }
	const RAIICounter c_Playback(m_PlaybackWaveData.GetThreadCounter());

	// get shared future from playabck data.
	std::future<void>& sharedFuture = m_PlaybackWaveData.GetFutureFromPlayback();

	// lamda { if wave buffer is full => true }
	const auto waveBufferStatus = [&]() -> const bool { return (bool)m_WaveBuffer->GetStatus(); };

	//--------------------------------------------------------------------------------------------------------//

	//---------------------------------------- Thread loop ---------------------------------------------------//

	// loop.
	while (true)
	{
		// lock wave buffer.
		std::unique_lock<std::mutex> lockWaveBuffer(m_WaveBuffer->m_BufferMutex);

		// wait till a notify is recieved and wave buffer is full.
		m_Wave_cv.wait(lockWaveBuffer, std::move(waveBufferStatus));

		// (if future is still valid) => continue playing.
		if (sharedFuture.valid())
		{
			// write to audio device.
			waveOutWrite(m_PlaybackWaveData.m_DeviceHandle, m_WaveHeader, sizeof(WAVEHDR));

			// set used size to zero.
			m_WaveBuffer->SetUsedSize(0u);

			// set buffer as empty.
			m_WaveBuffer->SetStatus(Buffer::Status::EMPTY);

			// notify playback thread. { write complete }
			m_PlaybackWaveData.m_Wave_To_Playback_cv.notify_one();
		}

		// if future is invalid, exit.
		else
		{
			// exit thread.
			break;
		}
	}

	//--------------------------------------------------------------------------------------------------------//

	//------------------------------------------ Thrad End ---------------------------------------------------//

	Debug::out("Wave_%u Closed!\n", m_WaveIndex);

	//--------------------------------------------------------------------------------------------------------//
}

// fill wave buffer.
void WaveBufferThread::FillBuffer(const uint8_t* const sourceBuffer, const uint32_t sourceSize)
{
	// lock wave buffer.
	std::lock_guard<std::mutex> lockWaveBuffer(m_WaveBuffer->m_BufferMutex);

	// reset wave buffer with zero.
	memset(m_WaveBuffer->GetBuffer(), 0, m_WaveBufferSize);

	// fill wave buffer.
	m_WaveBuffer->FillBuffer(sourceBuffer, sourceSize);

	//Debug::out("Coordinator --> Wave | Transfer Size:[%u]\n", sourceSize);
}

// set wave buffer status.
void WaveBufferThread::SetBufferStatus(const Buffer::Status newStatus)
{
	// { mutex protected access }
	m_WaveBuffer->SetStatus(newStatus);
}

// get wave buffer status.
const bool WaveBufferThread::GetBufferStatus() const
{
	// { mutex protected access }
	return (bool)m_WaveBuffer->GetStatus();
}

// get wave buffer mutex.
std::mutex& WaveBufferThread::GetBufferMutex() const
{
	return std::ref(m_WaveBuffer->m_BufferMutex);
}
