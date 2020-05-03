#pragma once

#define LISTEN_PORT	9001

// thread local storage에 해당 값들을 세팅해놓고, 어떤 thread인지 검사하는 용도로 사용
enum THREAD_TYPE
{
	THREAD_MAIN_ACCEPT,
	THREAD_IO_WORKER
};

extern __declspec(thread) int LThreadType;