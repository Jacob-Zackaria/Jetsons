#include "FileCoordinatorData.h"

// Default constructor.
FileCoordinatorData::FileCoordinatorData()
	:
	m_File_to_Coordinator_cv(),
	m_Coordinator_to_File_cv()
{
	// create temporary promise.
	std::promise<void> temporaryPromise;

	// get future from the promsie.
	m_FutureFileDone = temporaryPromise.get_future();

	// set promise value.
	temporaryPromise.set_value();
}

// Destructor.
FileCoordinatorData::~FileCoordinatorData()
{
}

