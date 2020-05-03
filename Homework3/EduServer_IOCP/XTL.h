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

// https://openmynotepad.tistory.com/57
// custom allocator는
// 아래의 typedef들을 정의하고(value_type만 필수인듯)
// rebind만 정의하면 됨

// https://openmynotepad.tistory.com/58?category=854742
// 구체적인 예시
template <class T>
class STLAllocator
{
public:
	// 생성자+소멸자는 default로! 다른 함수에서 해주게 됨
	STLAllocator() = default;

	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	// 복사 생성자 정의
	template <class U>
	STLAllocator(const STLAllocator<U>&)
	{}

	// 사용 중지 권고되었다 함
	template <class U>
	struct rebind
	{
		typedef STLAllocator<U> other;
	};

	// 생성자를 호출해주는! 받은 위치에 placement new를 호출해준다.
	////TODO: const T& t에는 뭐가 오는거지??
	void construct(pointer p, const T& t)
	{
		new(p)T(t);
	}

	// 소멸자를 호출
	void destroy(pointer p)
	{
		p->~T();
	}

	T* allocate(size_t n)
	{
		//TODO: 메모리풀에서 할당해서 리턴
		return static_cast<T*>(malloc(n*sizeof(T)));
	}

	void deallocate(T* ptr, size_t n)
	{
		//TODO: 메모리풀에 반납
		free(ptr);
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
	//TODO: STL 할당자를 사용하는 deque를 type으로 선언
	//typedef ... type;
};

template <class T>
struct xlist
{
	//TODO: STL 할당자 사용
	typedef std::list<T> type;
};

template <class K, class T, class C = std::less<K> >
struct xmap
{
	//TODO: STL 할당자 사용하는 map을  type으로 선언
	//typedef ... type;
};

template <class T, class C = std::less<T> >
struct xset
{
	//TODO: STL 할당자 사용하는 set을  type으로 선언
	//typedef ... type;
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
	//TODO: STL 할당자 사용하는 priority_queue을  type으로 선언
	//typedef ... type;
};

typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, STLAllocator<wchar_t>> xstring;

