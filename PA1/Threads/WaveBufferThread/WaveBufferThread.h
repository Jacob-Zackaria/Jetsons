#ifndef WAVE_BUFFER_THREAD_H
#define WAVE_BUFFER_THREAD_H

#include "SharedData/PlaybackWaveData.h"
#include "Buffer.h"

// used for thread functions.
using namespace ThreadFramework;

// Class for wave buffer thread.
class WaveBufferThread : public BannerBase
{
public:

	//--------Static constant data----------//

	// wave buffer size.
	static const uint32_t m_WaveBufferSize = 2u * 1024u;

	//-------------------------------------//

	//-----Constructors and Destructor-----//

	WaveBufferThread(const char* const threadName, PlaybackWaveData& playbackWaveData, const uint8_t newIndex);
	WaveBufferThread(const WaveBufferThread&) = delete;
	WaveBufferThread& operator = (const WaveBufferThread&) = delete;
	WaveBufferThread(WaveBufferThread&&) = default;
	WaveBufferThread& operator = (WaveBufferThread&&) = default;
	~WaveBufferThread();

	//------------------------------------//

	//---------Public methods-------------//

	// launch object as thread.
	void Launch();

	// function call operator.
	void operator ()();

	// fill wave buffer.
	void FillBuffer(const uint8_t* const sourceBuffer, const uint32_t sourceSize);

	// set wave buffer status.
	void SetBufferStatus(const Buffer::Status newStatus);

	// get wave buffer status.
	const bool GetBufferStatus() const;

	// get wave buffer mutex.
	std::mutex& GetBufferMutex() const;

	//------------------------------------//

	//-----------Public Data--------------//

	// condition_varibale for wave thread.
	std::condition_variable m_Wave_cv;

	//------------------------------------//

private:

	//----------------Data----------------//

	// thread.
	std::thread	m_WaveBufferThread;

	// buffer for wave thread.
	Buffer* m_WaveBuffer;

	// header for wave playback.
	WAVEHDR* m_WaveHeader;

	//playback - wave shared data.
	PlaybackWaveData& m_PlaybackWaveData;

	// wave index.
	uint8_t m_WaveIndex;

	//------------------------------------//
};


#endif WAVE_BUFFER_THREAD_H