#pragma once

LONG WINAPI ExceptionFilter(EXCEPTION_POINTERS* exceptionInfo) ;

// ������ �������� ������, ũ���ø� ��
inline void CRASH_ASSERT(bool isOk)
{
	if ( isOk )
		return ;

	int* crashVal = 0 ;
	*crashVal = 0xDEADBEEF ;
}
