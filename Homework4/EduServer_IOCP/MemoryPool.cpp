#include "stdafx.h"
#include "Exception.h"
#include "MemoryPool.h"

MemoryPool* GMemoryPool = nullptr;

// �� ����Ʈ¥�� pool�� ���ϴ��� ���ڷ� �ѱ�
SmallSizeMemoryPool::SmallSizeMemoryPool(DWORD allocSize) : mAllocSize(allocSize)
{
	////TODO: MEMORY_ALLOCATION_ALIGNMENT ���� �� Ŀ���ϳ�?
	CRASH_ASSERT(allocSize > MEMORY_ALLOCATION_ALIGNMENT);
	// �̰� ������ ó���� ȣ�����־��!
	InitializeSListHead(&mFreeList);
}

MemAllocInfo* SmallSizeMemoryPool::Pop()
{
	// pop�� �غ��µ�,
	// �� ó������ pop�ص� ��� �����Ƿ�, ���� �Ҵ��� �Ѵ�.
	// ���� �Ҵ��Ѱ� �ݳ��ÿ� push�ϸ鼭 �������� pool�� �޸𸮰� �߰���
	MemAllocInfo* mem = (MemAllocInfo*)InterlockedPopEntrySList(&mFreeList);
	if (NULL == mem)
	{
		// ���̻� �� �� �ִ� �޸𸮰� ������,
		////TODO: �̷��� �Ҵ��ϴµ�, _aligned_malloc�� MEMORY_ALLOCATION_ALIGNMENT�� �����ΰ�?
		// �켱 SLIST�� ������ ������ �̷��� ����� ��!
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
			mSmallSizeMemoryPoolTable[j] = pool; // Ư�� ������ ��� pool�� �����ع���

			// 1~32 : SmallSizeMemoryPool(32)
			// 33~64 : SmallSizeMemoryPool(64)
			// 65~96 : SmallSizeMemoryPool(96)
			// �̷�������, ���ϴ� ����Ʈ ũ�Ⱑ �־�����, Ư�� ũ���� pool�� ����ϵ���

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
	int realAllocSize = size + sizeof(MemAllocInfo); // Tũ�� + MemAllocInfo ũ��

	if (realAllocSize > MAX_ALLOC_SIZE) // 4096���� ũ��, �׳� �Ҵ���! �����͵��� �޸� ����ȭ�� ��������, ū�͵��� ��...
	{
		header = reinterpret_cast<MemAllocInfo*>(_aligned_malloc(realAllocSize, MEMORY_ALLOCATION_ALIGNMENT));
	}
	else
	{
		// ���� �ʿ��� ũ���� �޸�Ǯ���� pop!
		header = mSmallSizeMemoryPoolTable[realAllocSize]->Pop();
	}

	// ����� ���ܵ� pointer�� ���Ϲ���
	return AttachMemAllocInfo(header, realAllocSize);
}

void MemoryPool::Deallocate(void* ptr, long extraInfo)
{
	// ����� ���Ե� pointer�� ���Ϲ���
	MemAllocInfo* header = DetachMemAllocInfo(ptr);
	header->mExtraInfo = extraInfo; ///< �ֱ� �Ҵ翡 ���õ� ���� ��Ʈ
	
	long realAllocSize = InterlockedExchange(&header->mAllocSize, 0); ///< �ι� ���� üũ ����
	
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