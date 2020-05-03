#include "stdafx.h"
#include "IocpManager.h"
#include "EduServer_IOCP.h"
#include "ClientSession.h"
#include "SessionManager.h"

#define GQCS_TIMEOUT	20

// thread local storage에 선언. thread마다 번호를 붙임
__declspec(thread) int LIoThreadId = 0;
IocpManager* GIocpManager = nullptr;

IocpManager::IocpManager() : mCompletionPort(NULL), mIoThreadCount(2), mListenSocket(NULL)
{
}


IocpManager::~IocpManager()
{
}

bool IocpManager::Initialize()
{
	/// set num of I/O threads
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	mIoThreadCount = si.dwNumberOfProcessors;

	/// winsock initializing
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return false;

	/// Create I/O Completion Port
	mCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (mCompletionPort == NULL)
		return false;
	
	/// create TCP socket
	mListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (mListenSocket == INVALID_SOCKET)
		return false;

	// https://m.blog.naver.com/PostView.nhn?blogId=bringmelove1&logNo=119146643&proxyReferer=https:%2F%2Fwww.google.com%2F
	// 서버를 종료시켰다 다시 띄울 때, 이미 사용중인 소켓이라면서 에러나는 경우 방어
	int opt = 1;
	setsockopt(mListenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(int));

	/// bind
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(LISTEN_PORT);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (SOCKET_ERROR == bind(mListenSocket, (SOCKADDR*)&serveraddr, sizeof(serveraddr)))
		return false;

	return true;
}


bool IocpManager::StartIoThreads()
{
	/// I/O Thread
	for (int i = 0; i < mIoThreadCount; ++i)
	{
		DWORD dwThreadId;
		// i + 1 를 IoWorkerThread() 함수의 파라미터로 넘겨서, thread마다 번호를 붙여준다.
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, IoWorkerThread, (LPVOID)(i + 1), 0, (unsigned int*)&dwThreadId);
		if (hThread == NULL)
			return false;
	}

	return true;
}


bool IocpManager::StartAcceptLoop()
{
	/// listen
	if (SOCKET_ERROR == listen(mListenSocket, SOMAXCONN))
		return false;


	/// accept loop
	while (true)
	{
		SOCKET acceptedSock = accept(mListenSocket, NULL, NULL);
		if (acceptedSock == INVALID_SOCKET)
		{
			printf_s("accept: invalid socket\n");
			continue;
		}

		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(acceptedSock, (SOCKADDR*)&clientaddr, &addrlen);

		/// 소켓 정보 구조체 할당과 초기화
		ClientSession* client = GSessionManager->CreateClientSession(acceptedSock);

		/// 클라 접속 처리
		if (false == client->OnConnect(&clientaddr))
		{
			client->Disconnect(DR_ONCONNECT_ERROR);
			GSessionManager->DeleteClientSession(client);
		}
	}

	return true;
}

void IocpManager::Finalize()
{
	CloseHandle(mCompletionPort);

	/// winsock finalizing
	WSACleanup();

}


unsigned int WINAPI IocpManager::IoWorkerThread(LPVOID lpParam)
{
	LThreadType = THREAD_IO_WORKER;

	// thread마다 번호를 붙임
	LIoThreadId = reinterpret_cast<int>(lpParam);
	HANDLE hComletionPort = GIocpManager->GetComletionPort();

	while (true)
	{
		DWORD dwTransferred = 0;
		OverlappedIOContext* context = nullptr;
		ULONG_PTR completionKey = 0;
		ClientSession* asCompletionKey = nullptr;

		int ret = GetQueuedCompletionStatus(hComletionPort, &dwTransferred, (PULONG_PTR)&completionKey, (LPOVERLAPPED*)&context, GQCS_TIMEOUT);

		//ClientSession* theClient = context ? context->mSessionObject : nullptr;

		/// check time out first 
		if (ret == 0 && GetLastError()==WAIT_TIMEOUT)
			continue;

		if (ret == 0 || dwTransferred == 0)
		{
			/// connection closing
			asCompletionKey->Disconnect(DR_RECV_ZERO);
			GSessionManager->DeleteClientSession(asCompletionKey);
			continue;
		}

		// if (nullptr == context) 인 경우 처리
		//{
		//}

		bool completionOk = true;
		switch (context->mIoType)
		{
		case IO_SEND:
			completionOk = SendCompletion(asCompletionKey, context, dwTransferred);
			break;

		case IO_RECV:
			completionOk = ReceiveCompletion(asCompletionKey, context, dwTransferred);
			break;

		default:
			printf_s("Unknown I/O Type: %d\n", context->mIoType);
			break;
		}

		if ( !completionOk )
		{
			/// connection closing
			asCompletionKey->Disconnect(DR_COMPLETION_ERROR);
			GSessionManager->DeleteClientSession(asCompletionKey);
		}

	}

	return 0;
}

bool IocpManager::ReceiveCompletion(const ClientSession* client, OverlappedIOContext* context, DWORD dwTransferred)
{

	/// echo back 처리 client->PostSend()사용.
	
	delete context;

	return client->PostRecv();
}

bool IocpManager::SendCompletion(const ClientSession* client, OverlappedIOContext* context, DWORD dwTransferred)
{
	/// 전송 다 되었는지 확인하는 것 처리..
	//if (context->mWsaBuf.len != dwTransferred) {...}
	
	delete context;
	return true;
}
