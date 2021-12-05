#ifndef PLAYBACK_THREAD_H
#define PLAYBACK_THREAD_H

#include "SharedData/KillData.h"
#include "SharedData/CoordinatorPlaybackData.h"
#include "SharedData/PlaybackWaveData.h"
#include "WaveBufferThread/WaveBufferThread.h"

// used for thread functions.
using namespace ThreadFramework;

// Class for playback thread.
class PlaybackThread : public BannerBase
{
public:

	// user instance structure { used in callback }
	struct UserInstance
	{
		// default constructor.
		UserInstance()
			:
			m_DonePlaying(false),
			m_Closed(false),
			m_NumberOfWaves(0u)
		{
		}

		//----------------Data----------------//

		bool m_DonePlaying;
		bool m_Closed;
		uint8_t  m_NumberOfWaves;

		//------------------------------------//
	};

	//-----Constructors and Destructor-----//

	PlaybackThread(const char* const threadName, CoordinatorPlaybackData& coordinatorPlaybackData);
	PlaybackThread(const PlaybackThread&) = delete;
	PlaybackThread& operator = (const PlaybackThread&) = delete;
	PlaybackThread(PlaybackThread&&) = default;
	PlaybackThread& operator = (PlaybackThread&&) = default;
	~PlaybackThread();

	//------------------------------------//

	//---------Public methods-------------//

	// launch object as thread.
	void Launch(KillData& killData);

	// function call operator.
	void operator ()(KillData& killData);

	//------------------------------------//

private:

	//---------Private methods-------------//

	// call back function.
	static void CALLBACK waveOutProc(HWAVEOUT, UINT u_Message, DWORD dwInstance, DWORD dwParam1, DWORD);

	// send command to coordinator.
	void SendFillCommand(const uint8_t threadIndex);

	//------------------------------------//

	//----------------Data----------------//

	// playback-wave shared data. { original instance }
	static PlaybackWaveData* staticPlaybackWaveData;

	// thread.
	std::thread	m_PlaybackThread;

	// coordinator-playback shared data.
	CoordinatorPlaybackData& m_CoordinatorPlaybackData;

	// wave thread.
	WaveBufferThread* m_WaveThread[20];

	// User instance for callback.
	UserInstance m_UserInstance;

	//------------------------------------//
};



#endif PLAYBACK_THREAD_H