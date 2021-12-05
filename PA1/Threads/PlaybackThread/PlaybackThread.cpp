#include "PlaybackThread.h"
#include "ThreadCount/RAIICounter.h"

// Command files.
#include "Commands/C_LoadWaveBuffer.h"
#include "Commands/C_WaveDone.h"

// static declaration.
PlaybackWaveData* PlaybackThread::staticPlaybackWaveData = new PlaybackWaveData();

using namespace std::chrono_literals;

// Specialized Constructor.
PlaybackThread::PlaybackThread(const char* const threadName, CoordinatorPlaybackData& coordinatorPlaybackData)
	: BannerBase(threadName), m_PlaybackThread(),
	m_CoordinatorPlaybackData(coordinatorPlaybackData),
	m_UserInstance()
{
	// set waveform details.
	WAVEFORMATEX wfx = { 0 };
	wfx.nSamplesPerSec = 22050; // sample rate 
	wfx.wBitsPerSample = 16;    // number of bits per sample of mono data 
	wfx.nChannels = 2;          // number of channels (i.e. mono, stereo...) 
	wfx.wFormatTag = WAVE_FORMAT_PCM;								// format type 
	wfx.nBlockAlign = (WORD)((wfx.wBitsPerSample >> 3) * wfx.nChannels);	// block size of data 
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;		// for buffer estimation 
	wfx.cbSize = 0;

	// open device.
	MMRESULT result;
	result = waveOutOpen(&staticPlaybackWaveData->m_DeviceHandle, WAVE_MAPPER, &wfx, (DWORD_PTR)waveOutProc,
					(DWORD_PTR)&m_UserInstance, CALLBACK_FUNCTION);

	// verify device is open.
	assert(result == MMSYSERR_NOERROR);

	// temporary
	char waveName[16u];

	// create 20 wave threads
	for (uint8_t i = 0u; i < 20u; ++i)
	{
		// assign wave name.
		sprintf_s(waveName, 16u, "WaveThread_%d", i);

		// create new wave thread.
		m_WaveThread[i] = new WaveBufferThread(waveName, std::ref(*staticPlaybackWaveData), i);
	}
}

// Destructor.
PlaybackThread::~PlaybackThread()
{
	// destroy 20 wave threads.
	for (uint8_t i = 0u; i < 20u; ++i)
	{
		//delete wave thread.
		delete m_WaveThread[i];
	}

	// close wave device.
	waveOutClose(staticPlaybackWaveData->m_DeviceHandle);

	// wait until device is closed.
	while (!m_UserInstance.m_Closed)
	{
	};

	// delete playback-wave shared data.
	delete staticPlaybackWaveData;

	// check if joinable.
	if (m_PlaybackThread.joinable())
	{
		// join thread before destruction.
		m_PlaybackThread.join();
	}
}

// launch object as thread.
void PlaybackThread::Launch(KillData& killData)
{
	// check if not joinable.
	if (!m_PlaybackThread.joinable())
	{
		// spawn a new thread with 'this' object reference passed as callable object.
		m_PlaybackThread = std::thread(std::ref(*this), std::ref(killData));
	}
	else
	{
		// error. don't launch twice on same object.
		assert(false);
	}
}

// function call operator.
void PlaybackThread::operator()(KillData& killData)
{
	START_BANNER

	//--------------------------------------- Thread Start ---------------------------------------------------//
		
	// Mutex protected thread counter { Increments during construction, decrements during destruction }
	const RAIICounter c_Playback(killData.GetThreadCounter());

	// get shared future from kill data.
	std::future<void>& sharedFuture = killData.GetFutureFromShared();

	// Commands from wave callback.
	Command* inputCommand = nullptr;

	// wave index.
	uint8_t waveIndex = 0u;

	// lamda { if drainstatus is drained => true }
	const auto drainedStatus = [&]() -> const bool { return m_CoordinatorPlaybackData.GetDrainedStatus(); };

	// lambda { if wave buffer is empty => writeOut successfull. }
	const auto waveBufferStatus = [&]() -> const bool { return !(m_WaveThread[waveIndex]->GetBufferStatus()); };

	// Launch 20 wave threads and add 20 commands to fill.
	for (waveIndex = 0u; waveIndex < 20u; ++waveIndex)
	{
		// launch wave thread.
		m_WaveThread[waveIndex]->Launch();

		// send command to coordinator. { fill indexed wave thread }
		SendFillCommand(waveIndex);
	}

	// play the first 20 threads.
	for (waveIndex = 0u; waveIndex < 20u; ++waveIndex)
	{
		// set drained status.
		m_CoordinatorPlaybackData.SetDrainedStatus(CoordinatorPlaybackData::DrainStatus::NOT_DRAINED);

		// notify coordinator thread.
		m_CoordinatorPlaybackData.m_Playback_To_Coordinator_cv.notify_one();

		// lock playback thread.
		std::unique_lock<std::mutex> lockPlayback(m_CoordinatorPlaybackData.m_CoordinatorPlaybackMutex);

		// wait for coordinator notify.
		m_CoordinatorPlaybackData.m_Coordinator_To_Playback_cv.wait(lockPlayback, std::move(drainedStatus));

		// notify wave thread.
		m_WaveThread[waveIndex]->m_Wave_cv.notify_one();
		
		// lock playback thread.
		std::unique_lock<std::mutex> lockPlaybackForWave(m_WaveThread[waveIndex]->GetBufferMutex());

		// wait for wave-thread to write. { wave buffer is empty }
		staticPlaybackWaveData->m_Wave_To_Playback_cv.wait(lockPlaybackForWave, std::move(waveBufferStatus));

		// increment counter.
		++m_UserInstance.m_NumberOfWaves;
	}

	//--------------------------------------------------------------------------------------------------------//

	//---------------------------------------- Thread loop ---------------------------------------------------//

	// (if future is still valid) => loop.
	while (sharedFuture.valid())
	{
		// if coordinator is still running.
		if (m_CoordinatorPlaybackData.m_FutureCoordinatorDone.valid())
		{
			// if there are commands from callback.
			if (staticPlaybackWaveData->m_Wave_to_Playback_Queue->PopFront(inputCommand))
			{
				// make sure command is available.
				assert(inputCommand);

				// get index of wave done.
				waveIndex = ((C_WaveDone*)inputCommand)->m_WaveIndex;

				// send command to coordinator. { fill indexed wave thread }
				SendFillCommand(waveIndex);

				// recycle command. { cleans and adds back to pool }
				staticPlaybackWaveData->m_PlaybackCommandPool->AddBackToPool(inputCommand);

				// set drained status.
				m_CoordinatorPlaybackData.SetDrainedStatus(CoordinatorPlaybackData::DrainStatus::NOT_DRAINED);

				// notify coordinator thread.
				m_CoordinatorPlaybackData.m_Playback_To_Coordinator_cv.notify_one();

				// lock playback thread.
				std::unique_lock<std::mutex> lockPlayback(m_CoordinatorPlaybackData.m_CoordinatorPlaybackMutex);

				// wait for coordinator notify.
				m_CoordinatorPlaybackData.m_Coordinator_To_Playback_cv.wait(lockPlayback, std::move(drainedStatus));

				// notify wave thread.
				m_WaveThread[waveIndex]->m_Wave_cv.notify_one();

				// lock playback thread.
				std::unique_lock<std::mutex> lockPlaybackForWave(m_WaveThread[waveIndex]->GetBufferMutex());

				// wait for wave-thread to write. { wave buffer is empty }
				staticPlaybackWaveData->m_Wave_To_Playback_cv.wait(lockPlaybackForWave, std::move(waveBufferStatus));

				// increment counter.
				++m_UserInstance.m_NumberOfWaves;
			}
		}

		// else if coordinator is done.
		else
		{
			// wait till all waves are done.
			while (!m_UserInstance.m_DonePlaying)
			{
			}

			// exit thread.
			break;
		}
		
	}

	//--------------------------------------------------------------------------------------------------------//

	//------------------------------------------ Thrad End ---------------------------------------------------//

	Debug::out("Kill Initiated.\n");

	// initiate kill. { for kill thread }
	killData.InitiateKill();

	// close all 20 wave threads. { makes future invalid }
	staticPlaybackWaveData->NotifyThreadsToExit();

	// set wave buffers as full, to wake up from wait state and exit.
	for (waveIndex = 0u; waveIndex < 20u; ++waveIndex)
	{
		// set wave as full
		m_WaveThread[waveIndex]->SetBufferStatus(Buffer::Status::FULL);

		// notify to exit
		m_WaveThread[waveIndex]->m_Wave_cv.notify_one();
	}

	// wait until all wave threads exit.
	staticPlaybackWaveData->WaitTillKillDone();

	Debug::out("DONE! All waves played.\n");

	//--------------------------------------------------------------------------------------------------------//
}

// call back function.
void PlaybackThread::waveOutProc(HWAVEOUT, UINT u_Message, DWORD dwInstance, DWORD dwParam1, DWORD)
{
	// user instance.
	UserInstance* userInstance = (UserInstance*)dwInstance;

	// wave header.
	WAVEHDR* waveHeader = nullptr;

	// switch
	switch (u_Message)
	{
	case WOM_DONE:

		// with this case, dwParam1 is the wavehdr
		waveHeader = (WAVEHDR*)dwParam1;

		// decrement wave count.
		--userInstance->m_NumberOfWaves;

		// if all waves finished.
		if (!userInstance->m_NumberOfWaves)
		{
			userInstance->m_DonePlaying = true;
		}
		else
		{
			// get wave done command from command pool.
			C_WaveDone* outputCommand = (C_WaveDone*)staticPlaybackWaveData->m_PlaybackCommandPool->GetCommandFromPool();

			// set wave index.
			outputCommand->m_WaveIndex = *((uint8_t*)waveHeader->dwUser);

			// push to queue, wave done command.
			staticPlaybackWaveData->m_Wave_to_Playback_Queue->PushBack(outputCommand);
		}

		//Debug::out("Wave_%u Done || Remain:%d \n", *((uint8_t*)waveHeader->dwUser), userInstance->m_NumberOfWaves);
		break;

	case WOM_CLOSE:

		// device closed. { using waveOutClose() }
		userInstance->m_Closed = true;
		Debug::out("WOM_CLOSE:\n");
		break;

	case WOM_OPEN:
		// device opened. { using waveOutOpen() }
		Debug::out("WOM_OPEN:\n");
		break;

	default:
		assert(false);
	}
}

// send command to coordinator.
void PlaybackThread::SendFillCommand(const uint8_t threadIndex)
{
	// get load command from command pool.
	C_LoadWaveBuffer* outputCommand = (C_LoadWaveBuffer*)m_CoordinatorPlaybackData.m_CoordinatorCommandPool->GetCommandFromPool();

	// set destination pointer.
	outputCommand->m_WaveThread = m_WaveThread[threadIndex];

	// push to queue, load wave command.
	m_CoordinatorPlaybackData.m_Play_to_Coordinator_Queue->PushBack(outputCommand);
}

