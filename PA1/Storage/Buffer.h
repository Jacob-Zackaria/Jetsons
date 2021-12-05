#ifndef BUFFER_H
#define BUFFER_H

// class to store buffer data.
class Buffer
{
public:

	//----------------Enums----------------//

	enum class Status
	{
		EMPTY,
		FULL
	};

	//------------------------------------//

	//-----Constructors and Destructor-----//

	Buffer() = delete;
	Buffer(uint32_t size);
	Buffer(const Buffer&) = delete;
	Buffer& operator = (const Buffer&) = delete;
	Buffer(Buffer&&) = default;
	Buffer& operator = (Buffer&&) = default;
	~Buffer();

	//------------------------------------//

	//---------Public methods-------------//

	// get start address.
	uint8_t* const GetBuffer() const;

	// return status of buffer.
	const Buffer::Status GetStatus();

	// set status of buffer.
	void SetStatus(const Buffer::Status status);

	// set used size.
	void SetUsedSize(const uint32_t size);

	// get used size.
	const uint32_t GetUsedSize() const;

	// get block with a max limit.
	const uint32_t GetBlock(const uint32_t maxLimit);

	// get total size.
	const uint32_t GetTotalSize() const;

	// fill buffer.
	void FillBuffer(const uint8_t* const sourceBuffer, const uint32_t sourceSize);

	//------------------------------------//

	//---------Public Data----------------//

	// mutex for buffer.
	std::mutex m_BufferMutex;

	//------------------------------------//

private:

	//----------------Data----------------//

	// buffer data. { unsigned char array }
	uint8_t* m_Buffer;

	// buffer status.
	Buffer::Status m_Status;

	// mutex for buffer status.
	std::mutex m_BufferStatusMutex;

	// total size of buffer.
	uint32_t m_TotalSize;

	// used size of buffer.
	uint32_t m_UsedSize;

	// buffer start address.
	uint8_t* m_StartLocation;

	// buffer end address.
	uint8_t* m_EndLocation;

	//------------------------------------//

};



#endif BUFFER_H