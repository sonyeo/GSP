#pragma once

// MemoryPool에서
// 4096바이트까지 모든 바이트에 대해서,
// 구간을 나눠서, 각 구간별로 SmallSizeMemoryPool를 1개씩 만들고
// 그걸 그 구간에 연결해놓음(ex. 1~32바이트 => SmallSizeMemoryPool(32))
// SmallSizeMemoryPool 안에서는, 처음 메모리를 얻어가면 새로 할당해서 계속 주고,
// 반납이되면, delete 하지 않고 풀에 계속 쌓아 놓음
// SmallSizeMemoryPool 안에선 MemAllocInfo를 가르키는 pointer를 SmallSizeMemoryPool의 담당 크기(+MemAllocInfo크기)만큼 할당해서 리턴하고
// MemoryPool을 통해 이 pointer를 리턴받을 땐, MemAllocInfo가 담긴 주소를 건너뛰고, 그 뒷 주소를 리턴(그러면 딱 담당크기(요청한 크기))만큼의 공간이 나옴


// SmallSizeMemoryPool 에서 사용할 구조체로,
// SLIST_ENTRY이며, SmallSizeMemoryPool에 SLIST_HEADER도 있음
/// 커스텀하게 힙에서 할당 받는 애들은 전부 메모리 정보 붙여주기
__declspec(align(MEMORY_ALLOCATION_ALIGNMENT))
struct MemAllocInfo : SLIST_ENTRY
{
	MemAllocInfo(int size) : mAllocSize(size), mExtraInfo(-1)
	{}
	
	long mAllocSize; ///< MemAllocInfo가 포함된 크기
	long mExtraInfo; ///< 기타 추가 정보 (예: 타입 관련 정보 등)

}; ///< total 16 바이트

// 메모리를 header가 포함된 크기로 받은 후, 이 함수를 호출
inline void* AttachMemAllocInfo(MemAllocInfo* header, int size)
{
	// 우선 할당받은 메모리를 가르키는 pointer라 header인데,
	// placement new임! header에 실제 MemAllocInfo의 생성자를 size를 인자로 해서 불러줌
	new (header)MemAllocInfo(size);
	// MemAllocInfo type의 포인터를 +1 해주었으므로,
	// 헤더의 뒤 위치를 가르키는 pointer를 리턴
	return reinterpret_cast<void*>(++header);
}

inline MemAllocInfo* DetachMemAllocInfo(void* ptr)
{
	// 위에서 리턴시에 헤더 뒤쪽 pointer를 주었었으므로,
	// 여기서는 받은 pointer를 MemAllocInfo type으로 캐스팅 한 후,
	MemAllocInfo* header = reinterpret_cast<MemAllocInfo*>(ptr);
	// -1을 해서, 헤더가 포함된 pointer를 리턴
	--header;
	return header;
}

// MemoryPool class에서 사용할 class
__declspec(align(MEMORY_ALLOCATION_ALIGNMENT))
class SmallSizeMemoryPool
{
public:
	SmallSizeMemoryPool(DWORD allocSize);

	MemAllocInfo* Pop();
	void Push(MemAllocInfo* ptr);
	

private:
	SLIST_HEADER mFreeList; ///< 반드시 첫번째 위치

	// 얼마 크기의 메모리인가?
	const DWORD mAllocSize;
	volatile long mAllocCount = 0;
};

class MemoryPool
{
public:
	MemoryPool();

	void* Allocate(int size);
	void Deallocate(void* ptr, long extraInfo);

private:
	enum Config
	{
		/// 함부로 바꾸면 안됨. 철저히 계산후 바꿀 것
		MAX_SMALL_POOL_COUNT = 1024/32 + 1024/128 + 2048/256, ///< ~1024까지 32단위, ~2048까지 128단위, ~4096까지 256단위
		MAX_ALLOC_SIZE = 4096
	};

	/// 원하는 크기의 메모리를 가지고 있는 풀에 O(1) access를 위한 테이블
	// 각 단위별로 미리 할당된 메모리가 들어있는= 테이블
	SmallSizeMemoryPool* mSmallSizeMemoryPoolTable[MAX_ALLOC_SIZE+1];

};

extern MemoryPool* GMemoryPool;


/// 요놈을 상속 받아야만 xnew/xdelete 사용할 수 있게...
struct PooledAllocatable {};


template <class T, class... Args>
T* xnew(Args... arg)
{
	static_assert(true == std::is_convertible<T, PooledAllocatable>::value, "only allowed when PooledAllocatable");

	// xnew 를 사용하면, T크기만큼 메모리풀을 만들고,
	void* alloc = GMemoryPool->Allocate(sizeof(T));
	new (alloc)T(arg...);
	return reinterpret_cast<T*>(alloc);
}

template <class T>
void xdelete(T* object)
{
	static_assert(true == std::is_convertible<T, PooledAllocatable>::value, "only allowed when PooledAllocatable");

	object->~T();
	GMemoryPool->Deallocate(object, sizeof(T));
	
}