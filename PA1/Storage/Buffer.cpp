#include "Buffer.h"

// Specialized Constructor.
Buffer::Buffer(uint32_t size)
	: m_BufferMutex(), 
	m_Buffer(new uint8_t[size]()),
	m_Status(Buffer::Status::EMPTY),
	m_BufferStatusMutex(),
	m_TotalSize(size),
	m_UsedSize(0u),
	m_StartLocation(m_Buffer),
	m_EndLocation(m_StartLocation)
{
}

// Destructor.
Buffer::~Buffer()
{
	delete[] m_Buffer;
}

// get start address.
uint8_t* const Buffer::GetBuffer() const
{
	return m_StartLocation;
}

// Get status of buffer.
const Buffer::Status Buffer::GetStatus()
{
	// lock buffer status.
	std::lock_guard<std::mutex> lockBufferStatus(m_BufferStatusMutex);

	// return status.
	return m_Status;
}

// Set status of buffer.
void Buffer::SetStatus(const Buffer::Status status)
{
	// lock buffer status.
	std::lock_guard<std::mutex> lockBufferStatus(m_BufferStatusMutex);

	// set status.
	m_Status = status;
}

// get used size.
const uint32_t Buffer::GetUsedSize() const
{
	return m_UsedSize;
}

// set used size.
void Buffer::SetUsedSize(const uint32_t size)
{
	// check if given size is less than total buffer size.
	assert(size <= m_TotalSize);

	// fix end address.
	m_EndLocation = m_StartLocation + size;

	// set used size.
	m_UsedSize = size;
}

// get block with a max limit.
const uint32_t Buffer::GetBlock(const uint32_t maxLimit)
{
	// size to return.
	uint32_t returnSize = maxLimit;

	// if limit is less than used size.
	if (maxLimit <= m_UsedSize)
	{
		// change used size.
		m_UsedSize -= maxLimit;
	}

	// else if limit is greater than used size.
	else
	{
		// change return size to what is left.
		returnSize = m_UsedSize;

		// change used size.
		m_UsedSize = 0u;
	}

	// change starting address.
	m_StartLocation += returnSize;

	// if start reached end.
	if (m_StartLocation >= m_EndLocation)
	{
		// set status as empty.
		this->SetStatus(Buffer::Status::EMPTY);

		// reset start location.
		m_StartLocation = m_Buffer;
	}

	return returnSize;
}

// get total size.
const uint32_t Buffer::GetTotalSize() const
{
	return m_TotalSize;
}

// fill buffer.
void Buffer::FillBuffer(const uint8_t* const sourceBuffer, const uint32_t sourceSize)
{
	// memory copy
	memcpy_s(m_StartLocation, m_TotalSize, sourceBuffer, sourceSize);

	// set buffer used size.
	SetUsedSize(sourceSize);

	// set buffer status. { mutex protected access. }
	SetStatus(Buffer::Status::FULL);
}
