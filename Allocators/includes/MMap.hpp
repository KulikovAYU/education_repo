//https://ru.stackoverflow.com/questions/419395/Как-написать-свой-аллокатор
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#include <tchar.h>
#endif 

#include <cassert>

#if defined(UNICODE) || defined(_UNICODE)
#define tcin std::wcin
#define tcout std::wcout
#else
#define tcin std::cin
#define tcout std::cout
#endif


template<size_t>
class PageWinPool;

template<size_t>
class PageLinuxPool;

#ifdef _WIN32
template<size_t PageSize>
using MemPool = PageWinPool<PageSize>;
// windows code goes here
#elif __linux__ 
template<size_t PageSize>
using MemPool = typename PageLinuxPool<PageSize>;
//linux code goes here
#endif 


//страж
#ifdef _WIN64
constexpr size_t BigAllocationSentinel = 0xFAFAFAFAFAFAFAFAULL;
#else // ^^^ _WIN64 ^^^ // vvv !_WIN64 vvv
constexpr size_t BigAllocationSentinel = 0xFAFAFAFAUL;
#endif // _WIN64

//служебная информация блока
#ifdef _DEBUG
constexpr size_t NonUserSize = 2*sizeof(void*); //2 - т.к. храним стража
#else // _DEBUG
constexpr size_t NonUserSize = sizeof(void*); //1 - т.к. стража нет
#endif // _DEBUG

//округление вверх до числа, кратного степени 2
inline size_t align(size_t x, size_t a) { return ((x - 1) | (a - 1)) + 1; }

//https://habr.com/ru/post/148657/
//https://habr.com/ru/post/505632/ - маст хев

/// <summary>
/// Пул больших блоков (страниц) [служебная информация(указатель на след. блок)][начало блока]
/// </summary>
template<typename PoolImpl,size_t PageSize = 65536>
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


/// <summary>
/// linus mmap impl
/// </summary>
template<size_t PageSize = 65536>
class PageLinuxPool : public PagePool<PageLinuxPool<PageSize>>
{
public:
	using PagePool<PageWinPool<PageSize>>::GetPage;

	uintptr_t* GetPage(size_t size)
	{
		throw std::logic_error("This method is not impl");
		return nullptr;
	}

	~PageLinuxPool() {}
};


//Пул блоков заданного размера
template<typename T, size_t PageSize = 65536, size_t Aligment = sizeof(void*)>
class BlockPool : protected MemPool<PageSize>
{
public:
	BlockPool()
	{
		//блок должен вмещать тип и вдовесок полезную нагрузку
		m_blockSize = align(sizeof(T) + NonUserSize, Aligment);
		m_count = PageSize / m_blockSize;
	}

	void* Alloc(size_t countObjects)
	{
		if (countObjects == 0u)
			return nullptr;

		void* pTmpHead = nullptr;
		for (size_t i = 0; i < countObjects; ++i)
		{
			//если блок не инциализирован или же если инциализирован,и не осталось места в блоке
			//просим ОС выделить новую страницу
			//если осталось, то просто перемещаем указатель на новый сегмент
			bool bEndOrBeginOfPage = m_pHead ? (static_cast<uintptr_t*>(m_pHead)[-1] == NULL) : true;
			if (bEndOrBeginOfPage)
				FormatNewPage();

			void* tmp = m_pHead;
			//указатель на блок данных(блок данных - блок с полезной нагрузкой)
			void* next = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(tmp) + NonUserSize + m_blockSize);
			m_pHead = next;

			if (i == 0)
				pTmpHead = tmp;
		}
	

		return pTmpHead;
	}

	void Free(void* tmp, size_t countObjects)
	{
		//traverse back from list
		for (size_t i = 0; i < countObjects; ++i)
		{
			static_cast<uintptr_t*>(tmp)[-1] = reinterpret_cast<uintptr_t>(m_pHead);
			m_pHead = tmp;
		}

		#ifdef _DEBUG
		bool bIsOk = (static_cast<uintptr_t*>(m_pHead)[-2] == BigAllocationSentinel);
		assert(bIsOk && "Can't find BigAllocationSentinel flag");
		#endif // DEBUG
	}

private:
	void FormatNewPage()
	{
		//метод разбивает большой блок на сегменты, имеющие вид
		//[служ. информация(указатель на след блок)][блок данных]->[служ. информация(указатель на след блок)][блок данных]->nil


		void* tmp = this->GetPage();
		m_pHead = tmp;
		for (size_t i = 0; i < m_count - 1; ++i)
		{
			//указатель на начало предыдущего блока со служебной информацией
			uintptr_t ptrContainer = reinterpret_cast<uintptr_t>(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(tmp) - NonUserSize));

			//указатель в служебном поле на след блок данных
			void* next = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(tmp) + NonUserSize + m_blockSize);

			//служебная запись указателя на начало блока с полезными данными
			static_cast<uintptr_t*>(next)[-1] = ptrContainer;
			tmp = next;
		}

		//пометим конец блока
		static_cast<uintptr_t*>(tmp)[-1] = NULL;
	}


	size_t m_blockSize  = 0;			//размер одного блока для хранения типа T
	size_t m_count		= 0;			//кол-во блоков
	void*  m_pHead		= { nullptr };  //начало блока
};