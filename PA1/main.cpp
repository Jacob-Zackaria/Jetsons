//----------------------------------------------------------------------------
// Copyright 2021, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------

#include <conio.h>

using namespace ThreadFramework;
using namespace std::chrono_literals;

// user-thread files.
#include "FileThread/FileThread.h"
#include "CoordinatorThread/CoordinatorThread.h"
#include "PlaybackThread/PlaybackThread.h"
#include "KillThread/KillThread.h"

// Program start
int main()
{
	START_BANNER_MAIN("-Main-");

	//-----------Communication---------------//

	KillData killData;
	FileCoordinatorData fileCoordinatorData;
	CoordinatorPlaybackData coordinatorPlaybackData;

	//--------------------------------------//

	//-----------Object Creation------------//

	KillThread t_KillThread("Kill Thread");
	FileThread t_FileThread("File Thread", std::ref(fileCoordinatorData));
	CoordinatorThread t_CoordinatorThread("Coordinator Thread", std::ref(fileCoordinatorData), std::ref(coordinatorPlaybackData));
	PlaybackThread t_PlaybackThread("Playback Thread", std::ref(coordinatorPlaybackData));

	//------------------------------------//

	//-----------Spawn threads------------//

	// async launch.
	std::future<void> futureFromKillThread = std::async(std::launch::async, std::move(t_KillThread), std::ref(killData));

	// thread launch.
	t_FileThread.Launch(std::ref(killData));
	t_CoordinatorThread.Launch(std::ref(killData));
	t_PlaybackThread.Launch(std::ref(killData));

	//------------------------------------//

	// success. { calls futureFromKillThread.get() by default }
	return 0;
}
// Program end

// --- End of File ---
