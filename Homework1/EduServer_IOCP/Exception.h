#pragma once

LONG WINAPI ExceptionFilter(EXCEPTION_POINTERS* exceptionInfo) ;

// 조건을 만족하지 않으면, 크래시를 냄
inline void CRASH_ASSERT(bool isOk)
{
	if ( isOk )
		return ;

	int* crashVal = 0 ;
	*crashVal = 0xDEADBEEF ;
}
