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

//��������� ���������� �����
#ifdef _DEBUG
constexpr size_t NonUserSize = 2 * sizeof(void*); //2 - �.�. ������ ������
#else // _DEBUG
constexpr size_t NonUserSize = sizeof(void*); //1 - �.�. ������ ���
#endif // _DEBUG

//�����
#ifdef ENVIRONMENT64
constexpr size_t BigAllocationSentinel = 0xFAFAFAFAFAFAFAFAULL;
#else // ^^^ _WIN64 ^^^ // vvv !_WIN64 vvv
constexpr size_t BigAllocationSentinel = 0xFAFAFAFAUL;
#endif // _WIN64

// <summary>
/// ��� ������� ������ (�������) [��������� ����������(��������� �� ����. ����)][������ �����]
/// </summary>
template<typename PoolImpl, size_t PageSize = 65536>
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