#pragma once

#include "Session.h"
#include "Player.h"

class ClientSessionManager;

// �ٸ� Server���� ����� ���еǵ��� ClientSession�̶�� ����ϴ°� ���ƺ���
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
	// ClientSession���� Player�� ����� ��� ����
	Player			mPlayer;

private:
	
	SOCKADDR_IN		mClientAddr ;

	
	friend class ClientSessionManager;
} ;



