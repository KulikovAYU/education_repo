#pragma once

// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif


#if defined(UNICODE) || defined(_UNICODE)
#define tcin std::wcin
#define tcout std::wcout
#else
#define tcin std::cin
#define tcout std::cout
#endif

//служебная информация блока
#ifdef _DEBUG
constexpr size_t NonUserSize = 2 * sizeof(void*); //2 - т.к. храним стража
#else // _DEBUG
constexpr size_t NonUserSize = sizeof(void*); //1 - т.к. стража нет
#endif // _DEBUG

//страж
#ifdef ENVIRONMENT64
constexpr size_t BigAllocationSentinel = 0xFAFAFAFAFAFAFAFAULL;
#else // ^^^ _WIN64 ^^^ // vvv !_WIN64 vvv
constexpr size_t BigAllocationSentinel = 0xFAFAFAFAUL;
#endif // _WIN64

// <summary>
/// Пул больших блоков (страниц) [служебная информация(указатель на след. блок)][начало блока]
/// </summary>
template<typename PoolImpl, size_t PageSize = 65536>
class PagePool
{
public:

	void* GetPage()
	{
		//запишем в служебные поля блока для режима _DEBUG:[сторож;указатель на начало блока;полезные данные]
		//запишем в служебные поля блока для режима _RELEASE:[указатель на начало блока; полезные данные]

		//размер реального блока для выделения
		const size_t blockSize = NonUserSize + PageSize;
		void* pNewPage = GetPage(blockSize);

		//адрес начала блока как safety int
		const uintptr_t ptrNewPage = reinterpret_cast<uintptr_t>(pNewPage);

		//блок с полезными данными
		//смещаем указатель на величину NonUserSize
		void* const pRealDataBlock = reinterpret_cast<void*>(ptrNewPage + NonUserSize);

		//служебная запись указателя на начало блока
		static_cast<uintptr_t*>(pRealDataBlock)[-1] = ptrNewPage;

		#ifdef _DEBUG
		//служебная запись стража
		static_cast<uintptr_t*>(pRealDataBlock)[-2] = BigAllocationSentinel;
		#endif // _DEBUG

		return pRealDataBlock;
	}

	/*void* GetMemoryChunk(size_t size)
	{
		if (m_nUsedCount + size > m_rawMemSize)
			return nullptr;

		void* pMem = m_pHeadMemory + m_nUsedCount;
		m_nUsedCount += size;

		return pMem;
	}

	void FreeMemoryChunk(size_t size)
	{
		m_rawMemSize -= size;
	}*/

protected:
	void* GetPage(size_t size)
	{
		return static_cast<PoolImpl*>(this)->GetPage(size);
	}


protected:
	uintptr_t* m_pHeadMemory = { nullptr }; //начало блока
};