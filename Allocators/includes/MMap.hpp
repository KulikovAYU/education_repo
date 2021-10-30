#pragma once

#ifdef _WIN32
#include "WinMMap.hpp"
#elif __linux__ 
#include "LinuxMMap.hpp"
#endif


//округление вверх до числа, кратного степени 2
inline size_t align(size_t x, size_t a) { return ((x - 1) | (a - 1)) + 1; }


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


	size_t m_blockSize = 0; //размер одного блока для хранения типа T
	size_t m_count = 0; //кол-во блоков
	void* m_pHead = { nullptr }; //начало блока
};