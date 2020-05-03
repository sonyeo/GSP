#include "stdafx.h"
#include "FastSpinlock.h"
#include "MemoryPool.h"
#include "EduServer_IOCP.h"
#include "ClientSession.h"
#include "SessionManager.h"
#include "IocpManager.h"


SessionManager* GSessionManager = nullptr;

SessionManager::~SessionManager()
{
	for (auto it : mFreeSessionList)
	{
		xdelete(it);
	}
}

void SessionManager::PrepareSessions()
{
	CRASH_ASSERT(LThreadType == THREAD_MAIN);

	// 세션을 미리 만들어 놓고
	for (int i = 0; i < MAX_CONNECTION; ++i)
	{
		ClientSession* client = xnew<ClientSession>();
			
		mFreeSessionList.push_back(client);
	}
}





void SessionManager::ReturnClientSession(ClientSession* client)
{
	FastSpinlockGuard guard(mLock);

	CRASH_ASSERT(client->mConnected == 0 && client->mRefCount == 0);

	client->SessionReset();

	mFreeSessionList.push_back(client);

	++mCurrentReturnCount;
}

bool SessionManager::AcceptSessions()
{
	FastSpinlockGuard guard(mLock);

	// 아무도 accept 안했으면, mCurrentIssueCount가 MAX_CONNECTION만큼이었을 것이고,
	// retun이 된게 있으면, 다시 AcceptEx()호출하여야..
	// 이거 overflow 날 수 있을듯
	while (mCurrentIssueCount - mCurrentReturnCount < MAX_CONNECTION)
	{
		ClientSession* newClient = mFreeSessionList.back();
		mFreeSessionList.pop_back();

		++mCurrentIssueCount;

		newClient->AddRef(); ///< refcount +1 for issuing 
		
		// AcceptEx()를 호출해주는걸, Issue라고 하나봄
		if (false == newClient->PostAccept())
			return false;
	}


	return true;
}