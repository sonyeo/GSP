#pragma once

// MemoryPool����
// 4096����Ʈ���� ��� ����Ʈ�� ���ؼ�,
// ������ ������, �� �������� SmallSizeMemoryPool�� 1���� �����
// �װ� �� ������ �����س���(ex. 1~32����Ʈ => SmallSizeMemoryPool(32))
// SmallSizeMemoryPool �ȿ�����, ó�� �޸𸮸� ���� ���� �Ҵ��ؼ� ��� �ְ�,
// �ݳ��̵Ǹ�, delete ���� �ʰ� Ǯ�� ��� �׾� ����
// SmallSizeMemoryPool �ȿ��� MemAllocInfo�� ����Ű�� pointer�� SmallSizeMemoryPool�� ��� ũ��(+MemAllocInfoũ��)��ŭ �Ҵ��ؼ� �����ϰ�
// MemoryPool�� ���� �� pointer�� ���Ϲ��� ��, MemAllocInfo�� ��� �ּҸ� �ǳʶٰ�, �� �� �ּҸ� ����(�׷��� �� ���ũ��(��û�� ũ��))��ŭ�� ������ ����


// SmallSizeMemoryPool ���� ����� ����ü��,
// SLIST_ENTRY�̸�, SmallSizeMemoryPool�� SLIST_HEADER�� ����
/// Ŀ�����ϰ� ������ �Ҵ� �޴� �ֵ��� ���� �޸� ���� �ٿ��ֱ�
__declspec(align(MEMORY_ALLOCATION_ALIGNMENT))
struct MemAllocInfo : SLIST_ENTRY
{
	MemAllocInfo(int size) : mAllocSize(size), mExtraInfo(-1)
	{}
	
	long mAllocSize; ///< MemAllocInfo�� ���Ե� ũ��
	long mExtraInfo; ///< ��Ÿ �߰� ���� (��: Ÿ�� ���� ���� ��)

}; ///< total 16 ����Ʈ

// �޸𸮸� header�� ���Ե� ũ��� ���� ��, �� �Լ��� ȣ��
inline void* AttachMemAllocInfo(MemAllocInfo* header, int size)
{
	// �켱 �Ҵ���� �޸𸮸� ����Ű�� pointer�� header�ε�,
	// placement new��! header�� ���� MemAllocInfo�� �����ڸ� size�� ���ڷ� �ؼ� �ҷ���
	new (header)MemAllocInfo(size);
	// MemAllocInfo type�� �����͸� +1 ���־����Ƿ�,
	// ����� �� ��ġ�� ����Ű�� pointer�� ����
	return reinterpret_cast<void*>(++header);
}

inline MemAllocInfo* DetachMemAllocInfo(void* ptr)
{
	// ������ ���Ͻÿ� ��� ���� pointer�� �־������Ƿ�,
	// ���⼭�� ���� pointer�� MemAllocInfo type���� ĳ���� �� ��,
	MemAllocInfo* header = reinterpret_cast<MemAllocInfo*>(ptr);
	// -1�� �ؼ�, ����� ���Ե� pointer�� ����
	--header;
	return header;
}

// MemoryPool class���� ����� class
__declspec(align(MEMORY_ALLOCATION_ALIGNMENT))
class SmallSizeMemoryPool
{
public:
	SmallSizeMemoryPool(DWORD allocSize);

	MemAllocInfo* Pop();
	void Push(MemAllocInfo* ptr);
	

private:
	SLIST_HEADER mFreeList; ///< �ݵ�� ù��° ��ġ

	// �� ũ���� �޸��ΰ�?
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
		/// �Ժη� �ٲٸ� �ȵ�. ö���� ����� �ٲ� ��
		MAX_SMALL_POOL_COUNT = 1024/32 + 1024/128 + 2048/256, ///< ~1024���� 32����, ~2048���� 128����, ~4096���� 256����
		MAX_ALLOC_SIZE = 4096
	};

	/// ���ϴ� ũ���� �޸𸮸� ������ �ִ� Ǯ�� O(1) access�� ���� ���̺�
	// �� �������� �̸� �Ҵ�� �޸𸮰� ����ִ�= ���̺�
	SmallSizeMemoryPool* mSmallSizeMemoryPoolTable[MAX_ALLOC_SIZE+1];

};

extern MemoryPool* GMemoryPool;


/// ����� ��� �޾ƾ߸� xnew/xdelete ����� �� �ְ�...
struct PooledAllocatable {};


template <class T, class... Args>
T* xnew(Args... arg)
{
	static_assert(true == std::is_convertible<T, PooledAllocatable>::value, "only allowed when PooledAllocatable");

	// xnew �� ����ϸ�, Tũ�⸸ŭ �޸�Ǯ�� �����,
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