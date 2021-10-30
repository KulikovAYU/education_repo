#pragma once
#include "PagePool.hpp"
#include <cassert>


#define ALLOC_PROFILER_DEF

template<size_t>
class PageLinuxPool;

template<size_t PageSize>
using MemPool = typename PageLinuxPool<PageSize>;

/// <summary>
/// linus mmap impl
/// </summary>
template<size_t PageSize = 65536>
class PageLinuxPool : public PagePool<PageLinuxPool<PageSize>>
{
public:
	using PagePool<PageLinuxPool<PageSize>>::GetPage;

	uintptr_t* GetPage(size_t size)
	{
		throw std::logic_error("This method is not impl");
		return nullptr;
	}

	~PageLinuxPool() {}
};