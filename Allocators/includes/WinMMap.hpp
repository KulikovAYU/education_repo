#pragma once
#include <Windows.h>
#include <tchar.h>
#include "PagePool.hpp"
#include <cassert>


#define ALLOC_PROFILER_DEF __declspec(allocator)


template<size_t>
class PageWinPool;

template<size_t PageSize>
using MemPool = PageWinPool<PageSize>;


/// <summary>
/// win mmap impl
/// </summary>
template<size_t PageSize = 65536>
class PageWinPool : public PagePool<PageWinPool<PageSize>>
{
public:
	//для возможности вызова метода базового класса
	using PagePool<PageWinPool<PageSize>>::GetPage;

	void* GetPage(size_t size)
	{
		//https ://github.com/RIscRIpt/winapi_ex_mmap/blob/master/winapi_ex_mmap/main.cpp
		HANDLE hMapping = CreateFileMapping(
			INVALID_HANDLE_VALUE,			// hFile,
			NULL,							// lpAttributes,
			PAGE_READWRITE,					// flProtect,
			size >> 32,						// dwMaximumSizeHigh,
			size & 0xFFFFFFFF,				// dwMaximumSizeLow,
			_T("ALLOC_SHARED_MEMORY_EX"));	// lpName


		if (!hMapping) {
			tcout << _T("Failed to create file mapping!") << std::endl
				<< _T("Error: ") << GetLastError() << std::endl;
		}
		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			tcout << _T("Opened already existing object.") << std::endl;
		}


		void* pPage = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (!pPage)
			tcout << _T("Failed to mapped view!");

		if (!this->m_pHeadMemory)
			this->m_pHeadMemory = static_cast<uintptr_t*>(pPage);

		CloseHandle(hMapping);

		return pPage;
	}

	~PageWinPool()
	{
		UnmapViewOfFile(this->m_pHeadMemory);
		//CloseHandle(m_hMapping);//подумать про деинициализацию
	}
};
