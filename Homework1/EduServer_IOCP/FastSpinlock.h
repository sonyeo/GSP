#pragma once

class FastSpinlock
{
public:
	FastSpinlock();
	~FastSpinlock();

	void EnterLock();
	void LeaveLock();
	
private:
	FastSpinlock(const FastSpinlock& rhs);
	FastSpinlock& operator=(const FastSpinlock& rhs);

	// InterlockedExchange()를 통해서 이 변수를 사용해 lock을 걸 예정
	////TODO: volatile 을 꼭 써야하나?
	volatile long mLockFlag;
};

// 특정 scope에 들어가고 나갈때, 생성자&소멸자를 통해 lock을 자동으로 걸고 해제하기위한 class
class FastSpinlockGuard
{
public:
	FastSpinlockGuard(FastSpinlock& lock) : mLock(lock)
	{
		mLock.EnterLock();
	}

	~FastSpinlockGuard()
	{
		mLock.LeaveLock();
	}

private:
	FastSpinlock& mLock;
};

// class단위로 유일한 lock을 만드는 용도
////TODO: 어디에 사용할까?
template <class TargetClass>
class ClassTypeLock
{
public:
	struct LockGuard
	{
		LockGuard()
		{
			TargetClass::mLock.EnterLock();
		}

		~LockGuard()
		{
			TargetClass::mLock.LeaveLock();
		}

	};

private:
	static FastSpinlock mLock;
	
	//friend struct LockGuard;
};

template <class TargetClass>
FastSpinlock ClassTypeLock<TargetClass>::mLock;