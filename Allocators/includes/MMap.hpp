//https://ru.stackoverflow.com/questions/419395/���-��������-����-���������
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


//�����
#ifdef _WIN64
constexpr size_t BigAllocationSentinel = 0xFAFAFAFAFAFAFAFAULL;
#else // ^^^ _WIN64 ^^^ // vvv !_WIN64 vvv
constexpr size_t BigAllocationSentinel = 0xFAFAFAFAUL;
#endif // _WIN64

//��������� ���������� �����
#ifdef _DEBUG
constexpr size_t NonUserSize = 2*sizeof(void*); //2 - �.�. ������ ������
#else // _DEBUG
constexpr size_t NonUserSize = sizeof(void*); //1 - �.�. ������ ���
#endif // _DEBUG

//���������� ����� �� �����, �������� ������� 2
inline size_t align(size_t x, size_t a) { return ((x - 1) | (a - 1)) + 1; }

//https://habr.com/ru/post/148657/
//https://habr.com/ru/post/505632/ - ���� ���

/// <summary>
/// ��� ������� ������ (�������) [��������� ����������(��������� �� ����. ����)][������ �����]
/// </summary>
template<typename PoolImpl,size_t PageSize = 65536>
class PagePool
{
public:

	void* GetPage()
	{
		//������� � ��������� ���� ����� ��� ������ _DEBUG:[������;��������� �� ������ �����;�������� ������]
		//������� � ��������� ���� ����� ��� ������ _RELEASE:[��������� �� ������ �����; �������� ������]
		
		//������ ��������� ����� ��� ���������
		const size_t blockSize = NonUserSize + PageSize;
		void* pNewPage = GetPage(blockSize);
		
		//����� ������ ����� ��� safety int
		const uintptr_t ptrNewPage = reinterpret_cast<uintptr_t>(pNewPage);
		
		//���� � ��������� �������
		//������� ��������� �� �������� NonUserSize
		void* const pRealDataBlock = reinterpret_cast<void*>(ptrNewPage + NonUserSize);

		//��������� ������ ��������� �� ������ �����
		static_cast<uintptr_t*>(pRealDataBlock)[-1] = ptrNewPage;
	
		#ifdef _DEBUG
		//��������� ������ ������
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
	uintptr_t* m_pHeadMemory = { nullptr }; //������ �����
};


/// <summary>
/// win mmap impl
/// </summary>
template<size_t PageSize = 65536>
class PageWinPool : public PagePool<PageWinPool<PageSize>>
{
public:
	//��� ����������� ������ ������ �������� ������
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
		//CloseHandle(m_hMapping);//�������� ��� ���������������
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


//��� ������ ��������� �������
template<typename T, size_t PageSize = 65536, size_t Aligment = sizeof(void*)>
class BlockPool : protected MemPool<PageSize>
{
public:
	BlockPool()
	{
		//���� ������ ������� ��� � �������� �������� ��������
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
			//���� ���� �� �������������� ��� �� ���� ��������������,� �� �������� ����� � �����
			//������ �� �������� ����� ��������
			//���� ��������, �� ������ ���������� ��������� �� ����� �������
			bool bEndOrBeginOfPage = m_pHead ? (static_cast<uintptr_t*>(m_pHead)[-1] == NULL) : true;
			if (bEndOrBeginOfPage)
				FormatNewPage();

			void* tmp = m_pHead;
			//��������� �� ���� ������(���� ������ - ���� � �������� ���������)
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
		//����� ��������� ������� ���� �� ��������, ������� ���
		//[����. ����������(��������� �� ���� ����)][���� ������]->[����. ����������(��������� �� ���� ����)][���� ������]->nil


		void* tmp = this->GetPage();
		m_pHead = tmp;
		for (size_t i = 0; i < m_count - 1; ++i)
		{
			//��������� �� ������ ����������� ����� �� ��������� �����������
			uintptr_t ptrContainer = reinterpret_cast<uintptr_t>(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(tmp) - NonUserSize));

			//��������� � ��������� ���� �� ���� ���� ������
			void* next = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(tmp) + NonUserSize + m_blockSize);

			//��������� ������ ��������� �� ������ ����� � ��������� �������
			static_cast<uintptr_t*>(next)[-1] = ptrContainer;
			tmp = next;
		}

		//������� ����� �����
		static_cast<uintptr_t*>(tmp)[-1] = NULL;
	}


	size_t m_blockSize  = 0;			//������ ������ ����� ��� �������� ���� T
	size_t m_count		= 0;			//���-�� ������
	void*  m_pHead		= { nullptr };  //������ �����
};