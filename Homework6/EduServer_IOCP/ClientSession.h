#pragma once

#include "Session.h"
#include "Player.h"

class ClientSessionManager;

// 다른 Server로의 연결과 구분되도록 ClientSession이라고 명명하는게 좋아보임
class ClientSession : public Session, public ObjectPool<ClientSession>
{
public:
	ClientSession();
	virtual ~ClientSession();

	void SessionReset();

	bool PostAccept();
	void AcceptCompletion();
	
	virtual void OnDisconnect(DisconnectReason dr);
	virtual void OnRelease();

public:
	// ClientSession에서 Player를 멤버로 들고 있음
	Player			mPlayer;

private:
	
	SOCKADDR_IN		mClientAddr ;

	
	friend class ClientSessionManager;
} ;



