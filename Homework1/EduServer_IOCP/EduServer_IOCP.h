#pragma once

#define LISTEN_PORT	9001

// thread local storage�� �ش� ������ �����س���, � thread���� �˻��ϴ� �뵵�� ���
enum THREAD_TYPE
{
	THREAD_MAIN_ACCEPT,
	THREAD_IO_WORKER
};

extern __declspec(thread) int LThreadType;