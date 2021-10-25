//Идеи взять отсюда:
//https://github.com/moya-lang/Allocator/blob/master/Allocator.h
///https://codereview.stackexchange.com/questions/220113/c17-allocator-traits-implementation
//https://github.com/dasfex/ProgrammingNotes/blob/master/cpp/topics/allocators.md - описание аллокатора


//https://ru.stackoverflow.com/questions/419395/Как-написать-свой-аллокатор
#include <iostream>
#include "MMap.hpp"

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




const SIZE_T size = 4096;



//struct MemoryArea
//{
//	MemoryArea()
//	{
//		m_mapper.InitMemory(size, true);
//	}
//
//	[[nodiscard]] void* allocate(size_t n)
//	{
//		return m_mapper.GetMemoryChunk(n);
//	}
//
//
//	void deallocate(size_t n)
//	{
//		return m_mapper.FreeMemoryChunk(n);
//	}
//
//	MMapTraits::MMaper m_mapper;
//};


template<typename T>
struct CAllocator;

template <class Alloc>
struct AllocatorTraits
{
	using allocatorType = Alloc;
	using valueType = typename Alloc::valueType;

	using pointer = valueType*;
	using constPointer = const valueType*;
	using voidPointer = void*;
	using constVoidPointer = const void*;
	

	using sizeType = size_t;
	using sizeDifferenceType = ptrdiff_t;


	/*template<typename Other>
	using RebindAlloc = typename Allocator::rebind<Other>::other;

	template<typename Other>
	using RebindTraits = AllocatorTraits<Alloc<Other>>;*/


	[[nodiscard]] static constexpr pointer allocate(Alloc& al, const sizeType count)
	{
		return al.allocate(count);
	}

	[[nodiscard]] static constexpr pointer allocate(Alloc& al, const sizeType count, constVoidPointer hint) 
	{
		return allocate(al, count, hint, 0);
	}


	static constexpr void deallocate(Alloc& al, pointer p, sizeType count)
	{
		al.deallocate(p, count);
	}


	template<typename T, class... Args>
	static constexpr void construct(Alloc& al, T* ptr, Args&&... args)
	{
		al.construct(ptr, std::forward<Args>(args)...);
	}


	template<typename T>
	static constexpr void destroy(Alloc& al, T* ptr)
	{
		al.destroy(ptr);
	}


	[[nodiscard]] static constexpr sizeType max_size(const Alloc& al) noexcept
	{
		return al.max_size();
	}

};

template<typename T>
struct CAllocator
{
	template<typename> friend class CAllocator;

public:
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;

	template<typename T2>
	struct rebind {
		typedef CAllocator<T2> other;
	};

private:
	T* ptr;
	size_t currentSize, maxSize;

public:
	CAllocator() noexcept :
		ptr(nullptr),
		currentSize(0),
		maxSize(0) {
	}

	CAllocator(T* buffer, size_t size) noexcept :
		ptr(buffer),
		currentSize(0),
		maxSize(size) {
	}

	template <typename T2>
	explicit CAllocator(const CAllocator<T2>& other) noexcept :
		ptr(reinterpret_cast<T*>(other.ptr)),
		currentSize(other.currentSize),
		maxSize(other.maxSize) {
	}

	T* allocate(size_t n, const void* hint = nullptr) {
		T* pointer = ptr + currentSize;
		currentSize += n;
		return pointer;
	}

	void deallocate(T* p, size_t n) {
		currentSize -= n;
	}

	size_t capacity() const noexcept {
		return maxSize;
	}

	/*size_t max_size() const noexcept {
		return maxSize;
	}*/

	T* address(T& x) const noexcept {
		return &x;
	}

	const T* address(const T& x) const noexcept {
		return &x;
	}

	T* buffer() const noexcept {
		return ptr;
	}

	template <typename T2>
	CAllocator& operator=(const CAllocator<T2>& alloc) {
		return *this;
	}

	template <typename... Args>
	void construct(T* p, Args&&... args) {
		new (p) T(forward<Args>(args)...);
	}

	void destroy(T* p) {
		p->~T();
	}

	[[nodiscard]] constexpr size_type max_size() const noexcept
	{
		return 0;
	}


	template <typename T2>
	bool operator==(const CAllocator<T2>& other) const noexcept {
		return ptr == other.ptr;
	}

	template <typename T2>
	bool operator!=(const CAllocator<T2>& other) const noexcept {
		return ptr != other.ptr;
	}



	MemoryPool<size> m_area;
};


//template <typename T>
//class stack_allocator {
//	template<typename> friend class stack_allocator;
//
//public:
//	typedef size_t size_type;
//	typedef ptrdiff_t difference_type;
//	typedef T* pointer;
//	typedef const T* const_pointer;
//	typedef T& reference;
//	typedef const T& const_reference;
//	typedef T value_type;
//
//	template<typename T2>
//	struct rebind {
//		typedef stack_allocator<T2> other;
//	};
//
//private:
//	T* ptr;
//	size_t currentSize, maxSize;
//
//public:
//	stack_allocator() noexcept :
//		ptr(nullptr),
//		currentSize(0),
//		maxSize(0) {
//	}
//
//	stack_allocator(T* buffer, size_t size) noexcept :
//		ptr(buffer),
//		currentSize(0),
//		maxSize(size) {
//	}
//
//	template <typename T2>
//	explicit stack_allocator(const stack_allocator<T2>& other) noexcept :
//		ptr(reinterpret_cast<T*>(other.ptr)),
//		currentSize(other.currentSize),
//		maxSize(other.maxSize) {
//	}
//
//	T* allocate(size_t n, const void* hint = nullptr) {
//		T* pointer = ptr + currentSize;
//		currentSize += n;
//		return pointer;
//	}
//
//	void deallocate(T* p, size_t n) {
//		currentSize -= n;
//	}
//
//	size_t capacity() const noexcept {
//		return maxSize;
//	}
//
//	size_t max_size() const noexcept {
//		return maxSize;
//	}
//
//	T* address(T& x) const noexcept {
//		return &x;
//	}
//
//	const T* address(const T& x) const noexcept {
//		return &x;
//	}
//
//	T* buffer() const noexcept {
//		return ptr;
//	}
//
//	template <typename T2>
//	stack_allocator& operator=(const stack_allocator<T2>& alloc) {
//		return *this;
//	}
//
//	template <typename... Args>
//	void construct(T* p, Args&&... args) {
//		new (p) T(forward<Args>(args)...);
//	}
//
//	void destroy(T* p) {
//		p->~T();
//	}
//
//	template <typename T2>
//	bool operator==(const stack_allocator<T2>& other) const noexcept {
//		return ptr == other.ptr;
//	}
//
//	template <typename T2>
//	bool operator!=(const stack_allocator<T2>& other) const noexcept {
//		return ptr != other.ptr;
//	}
//};