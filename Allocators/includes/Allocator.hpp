#pragma once
#include <iostream>
#include <type_traits>
#include <limits>
#include "MMap.hpp"


//
#define NODISCARD_DEF [[nodiscard]]
#define CONSTEXPR_DEF constexpr


const size_t size = 4096;


template<typename T>
struct CAllocator;

template <class Alloc>
struct AllocatorTraits
{
	using allocator_type = Alloc;
	using value_type = typename Alloc::value_type;

	using pointer = value_type*;
	using const_pointer = const value_type*;
	using void_pointer = void*;
	using const_void_pointer = const void*;
	

	using size_type = size_t;
	using difference_type = ptrdiff_t;

	//TODO: разобрать
	/*using propagate_on_container_copy_assignment = false_type;
	using propagate_on_container_move_assignment = true_type;
	using propagate_on_container_swap = false_type;
	using is_always_equal = true_type;*/

	//template<class Other>
	//using rebind_alloc = allocator<Other>;

	//TODO: пока не используем
	/*template <class _Other>
	using rebind_traits = allocator_traits<allocator<Other>>;*/


	NODISCARD_DEF static CONSTEXPR_DEF ALLOC_PROFILER_DEF pointer allocate(Alloc& al, const size_type n)
	{
		return al.allocate(n);
	}

	NODISCARD_DEF static CONSTEXPR_DEF ALLOC_PROFILER_DEF pointer allocate(Alloc& al, const size_type n, const_void_pointer hint)
	{
		return allocate_(al, n, hint, 0);
	}


	NODISCARD_DEF static CONSTEXPR_DEF ALLOC_PROFILER_DEF void deallocate(Alloc& al, pointer p, size_type count)
	{
		al.deallocate(p, count);
	}


	template<typename T, class... Args>
	static CONSTEXPR_DEF void construct(Alloc& al, T* p, Args&&... args)
	{
		construct_(p, std::forward<Args>(args)...);
	}


	template<typename T>
	static CONSTEXPR_DEF void destroy(Alloc& al, T* p)
	{
		destroy_(al, p, 0);
	}


	NODISCARD_DEF static CONSTEXPR_DEF size_type max_size(const Alloc& al) noexcept
	{
		return max_size_(al, 0);
	}

	NODISCARD_DEF static Alloc select_on_container_copy_construction(const Alloc& rhs)
	{
		return soccc(rhs, 0);
	}

private:
	//sfinae impls

	static auto allocate_(Alloc& al, size_type n, const_void_pointer hint, int)
		-> decltype(al.allocate(n, hint), void(), std::declval<pointer>())
	{
		return al.allocate(n, hint);
	}

	static auto allocate_(Alloc& al, size_type n, const_void_pointer, long)
		->pointer
	{
		return al.allocate(n);
	}

	template <typename T, typename... Args>
	static auto construct_(Alloc& a, T* p, int, Args&&... args)
		-> decltype(a.construct(p, std::forward<Args>(args)...), void())
	{
		a.construct(p, std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	static void construct_(Alloc&, T* p, long, Args&&... args)
	{
		::new(static_cast<void*>(p)) T(std::forward<Args>(args)...);
	}

	template<typename T>
	static auto destroy_(Alloc& a, T* p, int)
		->decltype(a.destroy(p), void())
	{
		a.destroy(p);
	}

	template<typename T>
	static void destroy_(Alloc&, T* p, long) 
	{
		p->~T();
	}

	static auto max_size_(const Alloc& a, int) noexcept
		->decltype(a.max_size(), std::declval<size_type>()) 
	{
		return a.max_size();
	}

	static auto max_size_(const Alloc&, long) noexcept
		-> size_type
	{
		//see answer https://stackoverflow.com/questions/27442885/syntax-error-with-stdnumeric-limitsmax
		return (std::numeric_limits<size_type>::max)() / sizeof(value_type);
	}

	static auto soccc(const Alloc& rhs, int)
		-> decltype(rhs.select_on_container_copy_construction(), std::declval<Alloc>())
	{
		return rhs.select_on_container_copy_construction();
	}
	static auto soccc(const Alloc& rhs, long)
		-> Alloc
	{
		return rhs;
	}

};

template<typename T>
struct CAllocator
{
	template<typename> friend struct CAllocator;

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

	T* allocate(size_t countObjects) {
		return static_cast<T*>(m_area.Alloc(countObjects));
		
		/*T* pointer = ptr + currentSize;
		currentSize += n;
		return pointer;*/
	}

	void deallocate(T* p, size_t countObjects) {
		m_area.Free(p, countObjects);
	
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
		new (p) T(std::forward<Args>(args)...);
	}

	void destroy(T* p) {
		p->~T();
	}

	[[nodiscard]] constexpr size_type max_size() const noexcept
	{
		return 1000;
	}


	template <typename T2>
	bool operator==(const CAllocator<T2>& other) const noexcept {
		return ptr == other.ptr;
	}

	template <typename T2>
	bool operator!=(const CAllocator<T2>& other) const noexcept {
		return ptr != other.ptr;
	}



	BlockPool<T, size> m_area;
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