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
	// ������ ��� ���鼭, busy-wait
	for (int nloops = 0; ; nloops++)
	{
		// mLockFlag�� 1�� ����, �����ϸ� �������� 0�� ����
		if ( InterlockedExchange(&mLockFlag, 1) == 0 )
			return;
	
		UINT uTimerRes = 1;
		// https://www.sysnet.pe.kr/2/0/11063
		// ���� 64hz(1s/64hz = 0.015625s = 15.625ms)��Ȯ���� Ÿ�̸Ӹ�, 1ms ������ ���е��� �ٲ���
		timeBeginPeriod(uTimerRes); 
		// �ʹݿ� 1,2,3,4,...,9ms ��ŭ sleep�ϴٰ�, 10���ĺ��ʹ� 10ms ������
		Sleep((DWORD)min(10, nloops));
		// 1ms�� �����ߴ� �ֱ⸦ ����
		timeEndPeriod(uTimerRes);
	}

}

void FastSpinlock::LeaveLock()
{
	// ������, 0���� �ٲ�
	InterlockedExchange(&mLockFlag, 0);
}