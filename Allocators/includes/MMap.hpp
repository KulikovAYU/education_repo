//https://ru.stackoverflow.com/questions/419395/Как-написать-свой-аллокатор
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#include <tchar.h>
#endif 


#if defined(UNICODE) || defined(_UNICODE)
#define tcin std::wcin
#define tcout std::wcout
#else
#define tcin std::cin
#define tcout std::cout
#endif


template<size_t>
class WinMemoryPool;

template<size_t>
class LinuxMemoryPool;

#ifdef _WIN32
template<size_t growSize>
using MemPool = WinMemoryPool<growSize>;
// windows code goes here
#elif __linux__ 
template<size_t growSize>
using MemPool = typename LinuxMemoryPool<growSize>;
//linux code goes here
#endif 



template<size_t growSize>
class MemoryPool
{
public:
	uint8_t* InitMemory(bool bReadAndWrite)
	{
		m_rawMemSize = growSize;
		return InitMemory(m_rawMemSize, bReadAndWrite);
	}

	void* GetMemoryChunk(size_t size)
	{
		if (m_nUsedCount + size > m_rawMemSize)
			return nullptr;
		
		void* pMem = m_pMemory + m_nUsedCount;
		m_nUsedCount += size;
	
		return pMem;
	}

	void FreeMemoryChunk(size_t size)
	{
		m_rawMemSize -= size;
	}

	
protected:
	uint8_t* m_pMemory = { nullptr }; //начало блока
	size_t m_rawMemSize = { 0 }; //смещение относительно начала блока (конец)
	size_t m_nUsedCount = { 0 }; //текущее смещение относительно начала блока
};


/// <summary>
/// win mmap impl
/// </summary>
template<size_t growSize>
class WinMemoryPool : protected MemoryPool<growSize>
{
public:
	uint8_t* InitMemory(size_t size, bool bReadAndWrite)
	{
		//https ://github.com/RIscRIpt/winapi_ex_mmap/blob/master/winapi_ex_mmap/main.cpp
		m_hMapping = CreateFileMapping(
			INVALID_HANDLE_VALUE,			// hFile,
			NULL,							// lpAttributes,
			PAGE_READWRITE,					// flProtect,
			size >> 32,						// dwMaximumSizeHigh,
			size & 0xFFFFFFFF,				// dwMaximumSizeLow,
			_T("ALLOC_SHARED_MEMORY_EX"));	// lpName
	

		if (!m_hMapping) {
			tcout << _T("Failed to create file mapping!") << std::endl
				<< _T("Error: ") << GetLastError() << std::endl;
		}
		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			tcout << _T("Opened already existing object.") << std::endl;
		}

		this->m_pMemory = static_cast<uint8_t*>(MapViewOfFile(m_hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0));
		if (!this->m_pMemory)
			tcout << _T("Failed to mapped view!");

		return this->m_pMemory;

	}

	~WinMemoryPool()
	{
		UnmapViewOfFile(this->m_pMemory);
		CloseHandle(m_hMapping);
	}


	HANDLE m_hMapping;
};


/// <summary>
/// linus mmap impl
/// </summary>
template<size_t growSize>
class LinuxMemoryPool : protected MemoryPool<growSize>
{
public:
	uint8_t* InitMemory(size_t size, bool bReadAndWrite)
	{
		throw std::logic_error("This method is not impl");
		return nullptr;
	}

	~LinuxMemoryPool() {}
};