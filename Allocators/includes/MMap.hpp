#pragma once

#ifdef _WIN32
#include "WinMMap.hpp"
#elif __linux__ 
#include "LinuxMMap.hpp"
#endif


//���������� ����� �� �����, �������� ������� 2
inline size_t align(size_t x, size_t a) { return ((x - 1) | (a - 1)) + 1; }


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


	size_t m_blockSize = 0; //������ ������ ����� ��� �������� ���� T
	size_t m_count = 0; //���-�� ������
	void* m_pHead = { nullptr }; //������ �����
};