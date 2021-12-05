#ifndef FILE_COORDINATOR_DATA_H
#define FILE_COORDINATOR_DATA_H

// structure which is shared across file and coordinator threads.
struct FileCoordinatorData
{
	//-----Constructors and Destructor-----//

	FileCoordinatorData();
	FileCoordinatorData(const FileCoordinatorData&) = delete;
	FileCoordinatorData& operator = (const FileCoordinatorData&) = delete;
	FileCoordinatorData(FileCoordinatorData&&) = default;
	FileCoordinatorData& operator = (FileCoordinatorData&&) = default;
	~FileCoordinatorData();

	//------------------------------------//

	//------------Public Data-------------//

	// condition variable.
	std::condition_variable m_File_to_Coordinator_cv;

	// condition variable.
	std::condition_variable m_Coordinator_to_File_cv;

	// future for file done.
	std::future<void> m_FutureFileDone;

	//------------------------------------//

};


#endif FILE_COORDINATOR_DATA_H