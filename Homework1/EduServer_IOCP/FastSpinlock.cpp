#include "stdafx.h"
#include "FastSpinlock.h"


FastSpinlock::FastSpinlock() : mLockFlag(0)
{
}


FastSpinlock::~FastSpinlock()
{
}


void FastSpinlock::EnterLock()
{
	// 루프를 계속 돌면서, busy-wait
	for (int nloops = 0; ; nloops++)
	{
		// mLockFlag에 1을 대입, 성공하면 이전값인 0을 리턴
		if ( InterlockedExchange(&mLockFlag, 1) == 0 )
			return;
	
		UINT uTimerRes = 1;
		// https://www.sysnet.pe.kr/2/0/11063
		// 원래 64hz(1s/64hz = 0.015625s = 15.625ms)정확도인 타이머를, 1ms 단위의 정밀도로 바꿔줌
		timeBeginPeriod(uTimerRes); 
		// 초반엔 1,2,3,4,...,9ms 만큼 sleep하다가, 10이후부터는 10ms 쉬도록
		Sleep((DWORD)min(10, nloops));
		// 1ms로 설정했던 주기를 해제
		timeEndPeriod(uTimerRes);
	}

}

void FastSpinlock::LeaveLock()
{
	// 나갈때, 0으로 바꿈
	InterlockedExchange(&mLockFlag, 0);
}