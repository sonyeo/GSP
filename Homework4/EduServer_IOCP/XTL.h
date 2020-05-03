#pragma once
#include "MemoryPool.h"
#include <list>
#include <vector>
#include <deque>
#include <set>
#include <hash_set>
#include <hash_map>
#include <map>
#include <queue>

template <class T>
class STLAllocator
{
public:
	STLAllocator() = default;

	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	template <class U>
	STLAllocator(const STLAllocator<U>&)
	{}

	template <class U>
	struct rebind
	{
		typedef STLAllocator<U> other;
	};

	void construct(pointer p, const T& t)
	{
		// placement new! 받은 pointer에 생성자를 호출해줌(복사 생성자?)
		new(p)T(t);
	}

	void destroy(pointer p)
	{
		// replace delete 메모리를 해제하지는 않음
		p->~T();
	}

	// 이 아래의 메모리 할당/해제를 위한 풀을 만들었기에, 이런 custom allocator를 선언

	// 메모리 할당시!
	T* allocate(size_t n)
	{
		return static_cast<T*>(GMemoryPool->Allocate((int)n*sizeof(T)));
	}

	// 메모리 해제시!
	void deallocate(T* ptr, size_t n)
	{
		GMemoryPool->Deallocate(ptr, -1599);
	}
};


template <class T>
struct xvector
{
	typedef std::vector<T, STLAllocator<T>> type;
};

template <class T>
struct xdeque
{
	typedef std::deque<T, STLAllocator<T>> type;
};

template <class T>
struct xlist
{
	typedef std::list<T, STLAllocator<T>> type;
};

template <class K, class T, class C = std::less<K> >
struct xmap
{
	typedef std::map<K, T, C, STLAllocator<std::pair<K, T>> > type;
};

template <class T, class C = std::less<T> >
struct xset
{
	typedef std::set<T, C, STLAllocator<T> > type;
};

template <class K, class T, class C = std::hash_compare<K, std::less<K>> >
struct xhash_map
{
	typedef std::hash_map<K, T, C, STLAllocator<std::pair<K, T>> > type;
};

template <class T, class C = std::hash_compare<T, std::less<T>> >
struct xhash_set
{
	typedef std::hash_set<T, C, STLAllocator<T> > type;
};

template <class T, class C = std::less<std::vector<T>::value_type> >
struct xpriority_queue
{
	typedef std::priority_queue<T, std::vector<T, STLAllocator<T>>, C> type;
};

typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, STLAllocator<wchar_t>> xstring;

