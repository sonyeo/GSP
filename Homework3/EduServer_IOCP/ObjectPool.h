#pragma once


#include "Exception.h"
#include "FastSpinlock.h"


template <class TOBJECT, int ALLOC_COUNT = 100>
class ObjectPool
{
public:

	// https://en.cppreference.com/w/cpp/memory/new/operator_new
	// class�� operator new overloading�� �̷��� static���� ��!
	// https://lyb1495.tistory.com/48
	// void* ���� + ù��° ���� size_t
	static void* operator new(size_t objSize)
	{
		//TODO: TOBJECT Ÿ�� ������ lock ���

		// free list�� ���� ��������� �ʾҴٸ�,
		if (!mFreeList)
		{
			// sizeof() : ����Ʈ ����
			// ALLOC_COUNT��ŭ array�� �����
			mFreeList = new uint8_t[sizeof(TOBJECT)*ALLOC_COUNT];

			// ���� ��ġ
			uint8_t* pNext = mFreeList;
			// ������ ��ġ�� �����Ͱ� �ִٰ� �����ϰ� (uint8_t[]�� Ư�� ��ġ�� ����Ű�� �����Ͱ� ����ִٰ� ����)
			uint8_t** ppCurr = reinterpret_cast<uint8_t**>(mFreeList);

			for (int i = 0; i < ALLOC_COUNT - 1; ++i)
			{
				// ���� TOBJECT ��ġ�� ������ +1
				pNext += sizeof(TOBJECT);
				// ���� ��ġ�� �����Ͱ� ����ִٰ� �����ϰ�, �ش� �������� ����, ���� ��ġ�� ����Ű���� 
				*ppCurr = pNext;
				// ���� ��ġ�� �ٽ�, Ư�� ��ġ�� ����Ű�� �����Ͷ�� ����
				ppCurr = reinterpret_cast<uint8_t**>(pNext);
			}

			mTotalAllocCount += ALLOC_COUNT;
		}
		
		// ���� ��ġ
		uint8_t* pAvailable = mFreeList;
		// ������ ������ ���־��⿡, �̷��� �ϸ� ���� ��ġ�� ����!
		// ������ pAvailable�� �����Ͽ� �־, ���� ����ϱ� ������ �Ⱦ��� �����̱⿡, �̷��� ���� �ּҸ� ���� �뵵�� ���
		////TODO: �׳� mFreeList += sizeof(TOBJECT) �ϸ� �ȵǳ�..?
		// �ȵȴ�! ó�� �Ҵ��� ������� ������, ���߿� �ݳ�(delete��)�� �������� �ϱ⿡, pointer�� �� �� �ۿ� ����
		mFreeList = *reinterpret_cast<uint8_t**>(pAvailable);

		// new �Ҷ����� count �ø���
		++mCurrentUseCount;

		return pAvailable;
	}

	static void	operator delete(void* obj)
	{
		//TODO: TOBJECT Ÿ�� ������ lock ���

		// new �� �Ǿ����
		CRASH_ASSERT(mCurrentUseCount > 0);

		// count ������
		--mCurrentUseCount;

		// ���� �ݳ��Ǵ� obj(������)��
		// �Ǵٸ� �����͸� ����Ű�� �ִٰ� �����ϰ�, uint8_t**�� ĳ���� �� ��,
		// �� ���� ���� free list�� ����Ű���� �ϰ�,
		// ���� free list�� obj�� ����Ű���� �ϸ�,
		// �ᱹ, obj�� free list�� �� �տ� ������ �Ǵ� ����
		*reinterpret_cast<uint8_t**>(obj) = mFreeList;
		mFreeList = static_cast<uint8_t*>(obj);
	}


private:
	static uint8_t*	mFreeList;
	static int		mTotalAllocCount; ///< for tracing
	static int		mCurrentUseCount; ///< for tracing

};


template <class TOBJECT, int ALLOC_COUNT>
uint8_t* ObjectPool<TOBJECT, ALLOC_COUNT>::mFreeList = nullptr;

template <class TOBJECT, int ALLOC_COUNT>
int ObjectPool<TOBJECT, ALLOC_COUNT>::mTotalAllocCount = 0;

template <class TOBJECT, int ALLOC_COUNT>
int ObjectPool<TOBJECT, ALLOC_COUNT>::mCurrentUseCount = 0;


