#include "stdafx.h"
#include "Exception.h"
#include "MemoryPool.h"

MemoryPool* GMemoryPool = nullptr;

// 몇 바이트짜리 pool을 원하는지 인자로 넘김
SmallSizeMemoryPool::SmallSizeMemoryPool(DWORD allocSize) : mAllocSize(allocSize)
{
	////TODO: MEMORY_ALLOCATION_ALIGNMENT 보다 왜 커야하나?
	CRASH_ASSERT(allocSize > MEMORY_ALLOCATION_ALIGNMENT);
	// 이거 무조건 처음에 호출해주어야!
	InitializeSListHead(&mFreeList);
}

MemAllocInfo* SmallSizeMemoryPool::Pop()
{
	// pop을 해보는데,
	// 맨 처음에는 pop해도 든게 없으므로, 새로 할당을 한다.
	// 새로 할당한건 반납시에 push하면서 그제서야 pool에 메모리가 추가됨
	MemAllocInfo* mem = (MemAllocInfo*)InterlockedPopEntrySList(&mFreeList);
	if (NULL == mem)
	{
		// 더이상 쓸 수 있는 메모리가 없으면,
		////TODO: 이렇게 할당하는데, _aligned_malloc와 MEMORY_ALLOCATION_ALIGNMENT가 무엇인가?
		// 우선 SLIST를 쓰려면 무조건 이렇게 해줘야 함!
		mem = reinterpret_cast<MemAllocInfo*>(_aligned_malloc(mAllocSize, MEMORY_ALLOCATION_ALIGNMENT));
	}
	else
	{
		CRASH_ASSERT(mem->mAllocSize == 0);
	}

	InterlockedIncrement(&mAllocCount);
	return mem;
}

void SmallSizeMemoryPool::Push(MemAllocInfo* ptr)
{
	// push
	InterlockedPushEntrySList(&mFreeList, (PSLIST_ENTRY)ptr);
	InterlockedDecrement(&mAllocCount);
}

/////////////////////////////////////////////////////////////////////

MemoryPool::MemoryPool()
{
	memset(mSmallSizeMemoryPoolTable, 0, sizeof(mSmallSizeMemoryPoolTable));

	int recent = 0;

	for (int i = 32; i < 1024; i+=32)
	{
		SmallSizeMemoryPool* pool = new SmallSizeMemoryPool(i);
		for (int j = recent+1; j <= i; ++j)
		{
			mSmallSizeMemoryPoolTable[j] = pool; // 특정 범위를 모두 pool에 연결해버림

			// 1~32 : SmallSizeMemoryPool(32)
			// 33~64 : SmallSizeMemoryPool(64)
			// 65~96 : SmallSizeMemoryPool(96)
			// 이런식으로, 원하는 바이트 크기가 주어지면, 특정 크기의 pool을 사용하도록

		}
		recent = i;
	}

	for (int i = 1024; i < 2048; i += 128)
	{
		SmallSizeMemoryPool* pool = new SmallSizeMemoryPool(i);
		for (int j = recent + 1; j <= i; ++j)
		{
			mSmallSizeMemoryPoolTable[j] = pool;
		}
		recent = i;
	}

	for (int i = 2048; i <= 4096; i += 256)
	{
		SmallSizeMemoryPool* pool = new SmallSizeMemoryPool(i);
		for (int j = recent + 1; j <= i; ++j)
		{
			mSmallSizeMemoryPoolTable[j] = pool;
		}
		recent = i;
	}

}

void* MemoryPool::Allocate(int size)
{
	MemAllocInfo* header = nullptr;
	int realAllocSize = size + sizeof(MemAllocInfo); // T크기 + MemAllocInfo 크기

	if (realAllocSize > MAX_ALLOC_SIZE) // 4096보다 크면, 그냥 할당함! 작은것들이 메모리 파편화가 심해지지, 큰것들은 뭐...
	{
		header = reinterpret_cast<MemAllocInfo*>(_aligned_malloc(realAllocSize, MEMORY_ALLOCATION_ALIGNMENT));
	}
	else
	{
		// 실제 필요한 크기의 메모리풀에서 pop!
		header = mSmallSizeMemoryPoolTable[realAllocSize]->Pop();
	}

	// 헤더가 제외된 pointer를 리턴받음
	return AttachMemAllocInfo(header, realAllocSize);
}

void MemoryPool::Deallocate(void* ptr, long extraInfo)
{
	// 헤더가 포함된 pointer를 리턴받음
	MemAllocInfo* header = DetachMemAllocInfo(ptr);
	header->mExtraInfo = extraInfo; ///< 최근 할당에 관련된 정보 힌트
	
	long realAllocSize = InterlockedExchange(&header->mAllocSize, 0); ///< 두번 해제 체크 위해
	
	CRASH_ASSERT(realAllocSize> 0);

	if (realAllocSize > MAX_ALLOC_SIZE)
	{
		_aligned_free(header);
	}
	else
	{
		mSmallSizeMemoryPoolTable[realAllocSize]->Push(header);
	}
}