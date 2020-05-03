#include "stdafx.h"
#include "Exception.h"
#include <DbgHelp.h>

// https://sunhyeon.wordpress.com/2015/11/08/1899/
// dbghelp.lib 링크 + Linker/Debugging/Generate Debug Info = YES(/DEBUG) -> pdb 파일 생성
LONG WINAPI ExceptionFilter(EXCEPTION_POINTERS* exceptionInfo)
{
	// 디버깅 중이면, 구지 minidump를 남기지 않음
	if ( IsDebuggerPresent() )
		return EXCEPTION_CONTINUE_SEARCH ; // global unwind 수행X


	/// dump file 남기자.

	HANDLE hFile = CreateFileA("EasyServer_minidump.dmp", GENERIC_READ | GENERIC_WRITE, 
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ) ; 

	if ( ( hFile != NULL ) && ( hFile != INVALID_HANDLE_VALUE ) ) 
	{
		MINIDUMP_EXCEPTION_INFORMATION mdei ; 

		mdei.ThreadId           = GetCurrentThreadId() ; 
		mdei.ExceptionPointers  = exceptionInfo ; 
		mdei.ClientPointers     = FALSE ; 


		MINIDUMP_TYPE mdt = (MINIDUMP_TYPE)(MiniDumpWithPrivateReadWriteMemory | 
			MiniDumpWithDataSegs | MiniDumpWithHandleData | MiniDumpWithFullMemoryInfo | 
			MiniDumpWithThreadInfo | MiniDumpWithUnloadedModules ) ; 

		// mini dump는 이렇게 남기면 됨
		MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), 
			hFile, mdt, (exceptionInfo != 0) ? &mdei : 0, 0, NULL ) ; 

		CloseHandle( hFile ) ; 

	}
	else 
	{
		printf("CreateFile failed. Error: %u \n", GetLastError()) ; 
	}

	// global unwind 수행
	////TODO: 어떤 의미?
	return EXCEPTION_EXECUTE_HANDLER  ;
	
}