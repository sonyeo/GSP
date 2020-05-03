#pragma once


#include "Exception.h"
#include "FastSpinlock.h"


template <class TOBJECT, int ALLOC_COUNT = 100>
class ObjectPool
{
public:

	// https://en.cppreference.com/w/cpp/memory/new/operator_new
	// class의 operator new overloading은 이렇게 static으로 함!
	// https://lyb1495.tistory.com/48
	// void* 리턴 + 첫번째 인자 size_t
	static void* operator new(size_t objSize)
	{
		//TODO: TOBJECT 타입 단위로 lock 잠금

		// free list가 아직 만들어지지 않았다면,
		if (!mFreeList)
		{
			// sizeof() : 바이트 리턴
			// ALLOC_COUNT만큼 array를 만들고
			mFreeList = new uint8_t[sizeof(TOBJECT)*ALLOC_COUNT];

			// 시작 위치
			uint8_t* pNext = mFreeList;
			// 각각의 위치에 포인터가 있다고 생각하고 (uint8_t[]의 특정 위치를 가르키는 포인터가 들어있다고 생각)
			uint8_t** ppCurr = reinterpret_cast<uint8_t**>(mFreeList);

			for (int i = 0; i < ALLOC_COUNT - 1; ++i)
			{
				// 다음 TOBJECT 위치로 포인터 +1
				pNext += sizeof(TOBJECT);
				// 이전 위치에 포인터가 들어있다고 생각하고, 해당 포인터의 값을, 다음 위치를 가르키도록 
				*ppCurr = pNext;
				// 다음 위치가 다시, 특정 위치를 가르키는 포인터라고 생각
				ppCurr = reinterpret_cast<uint8_t**>(pNext);
			}

			mTotalAllocCount += ALLOC_COUNT;
		}
		
		// 지금 위치
		uint8_t* pAvailable = mFreeList;
		// 위에서 세팅을 해주었기에, 이렇게 하면 다음 위치가 나옴!
		// 어차피 pAvailable을 리턴하여 주어서, 직접 사용하기 전에는 안쓰는 공간이기에, 이렇게 다음 주소를 얻어내는 용도로 사용
		////TODO: 그냥 mFreeList += sizeof(TOBJECT) 하면 안되나..?
		// 안된다! 처음 할당은 순서대로 하지만, 나중에 반납(delete시)은 순서없이 하기에, pointer로 할 수 밖에 없음
		mFreeList = *reinterpret_cast<uint8_t**>(pAvailable);

		// new 할때마다 count 올리고
		++mCurrentUseCount;

		return pAvailable;
	}

	static void	operator delete(void* obj)
	{
		//TODO: TOBJECT 타입 단위로 lock 잠금

		// new 가 되었어야
		CRASH_ASSERT(mCurrentUseCount > 0);

		// count 내리고
		--mCurrentUseCount;

		// 지금 반납되는 obj(포인터)가
		// 또다른 포인터를 가르키고 있다고 생각하고, uint8_t**로 캐스팅 한 후,
		// 그 값을 현재 free list를 가르키도록 하고,
		// 현재 free list는 obj를 가르키도록 하면,
		// 결국, obj가 free list의 맨 앞에 끼어들게 되는 형식
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


