#include <process.h>
#include <cstdio>
#include <cstdarg>
#include "stdio_redirector.h"

stdio_redirector::stdio_redirector()
:_hThread(nullptr)
, _hChildProcess(nullptr)
, _dwWaitInterval(100)
, _hStdinWrite(nullptr)
, _hStdoutRead(nullptr)

{

}

stdio_redirector::~stdio_redirector()
{
	close();
}

bool stdio_redirector::open(LPCTSTR pszCmdLine)
{
	HANDLE hStdoutReadTmp = nullptr;
	HANDLE hStdoutWrite = nullptr, hStderrWrite = nullptr;
	HANDLE hStdinWriteTmp = nullptr;
	HANDLE hStdinRead = nullptr;
	SECURITY_ATTRIBUTES sa;

	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = nullptr;
	sa.bInheritHandle = TRUE;
	
	try{
		if(!::CreatePipe(&hStdoutReadTmp, &hStdoutWrite, &sa, 0))
			throw "error: failed to createpipe.";

		if(!::DuplicateHandle(GetCurrentProcess(), hStdoutWrite,
			GetCurrentProcess(), &hStderrWrite,
			0, TRUE, DUPLICATE_SAME_ACCESS))
		{
			throw "error: failed to duplicatehandle.";
		}

		if(!::CreatePipe(&hStdinRead, &hStdinWriteTmp, &sa, 0))
			throw "error: failed to createpipe.";

		if(!::DuplicateHandle(
			::GetCurrentProcess(), hStdoutReadTmp,
			::GetCurrentProcess(), &_hStdoutRead,
			0, FALSE,
			DUPLICATE_SAME_ACCESS))
		{
			throw "error: failed to duplicatehandle.";
		}

		if(!::DuplicateHandle(
			::GetCurrentProcess(), hStdinWriteTmp,
			::GetCurrentProcess(), &_hStdinWrite,
			0, FALSE,
			DUPLICATE_SAME_ACCESS))
		{
			throw "error: failed to duplicatehandle.";
		}

		destroy_handle(hStdoutReadTmp);
		destroy_handle(hStdinWriteTmp);

		if(!launch_child(pszCmdLine, hStdoutWrite, hStdinRead, hStderrWrite))
			throw "error: failed to launch child process";

		destroy_handle(hStdoutWrite);
		destroy_handle(hStdinRead);
		destroy_handle(hStderrWrite);

		_hEventExit = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		_hThread = ::CreateThread(NULL, 0, thread_output, this, 0, &_dwThreadId);

		if(!_hEventExit || !_hThread)
			throw "error: ";
	}
	catch(const char* e){
		::MessageBoxA(NULL, e, NULL, MB_ICONERROR);
		return false;
	}

	return true;
}

bool stdio_redirector::print(LPCTSTR fmt, ...)
{
	char buf[2048];
	va_list va;
	va_start(va, fmt);
	int len = vsnprintf(buf, sizeof(buf), fmt, va);
	va_end(va);

	DWORD dwWritten;
	return !!::WriteFile(_hStdinWrite, buf, len, &dwWritten, NULL);
}

BOOL stdio_redirector::launch_child(LPCTSTR pszCmdLine, HANDLE hStdOut, HANDLE hStdin, HANDLE hStdErr)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si = {0};

	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	si.hStdError = hStdErr;
	si.hStdInput = hStdin;
	si.hStdOutput = hStdOut;

	if(!::CreateProcess(
		NULL, (LPTSTR)pszCmdLine,
		NULL, NULL,
		TRUE,
		CREATE_NEW_CONSOLE,
		NULL, NULL,
		&si, &pi))
	{
		return false;
	}

	_hChildProcess = pi.hProcess;
	::CloseHandle(pi.hThread);

	return TRUE;
}

int stdio_redirector::redirect_stdout()
{
	for(;;) {
		DWORD dwAvail = 0;
		if(!::PeekNamedPipe(_hStdoutRead, NULL, 0, 0, &dwAvail, NULL))
			break;

		if(!dwAvail) return 1;

		char szOutput[1024];
		DWORD dwRead = 0;
		if(!::ReadFile(_hStdoutRead, szOutput, min(1023, dwAvail), &dwRead, NULL))
			break;

		szOutput[dwRead] = 0;
		write_stdout(szOutput);
	}

	DWORD dwError = ::GetLastError();
	if(dwError == ERROR_BROKEN_PIPE
		|| dwError == ERROR_NO_DATA)
	{
		write_stdout("子进程已结束");
		return 0;
	}

	return -1;
}

void stdio_redirector::destroy_handle(HANDLE& h)
{
	CloseHandle(h);
	h = NULL;
}

DWORD WINAPI stdio_redirector::thread_output(LPVOID lpUser)
{
	HANDLE handles[2];
	int r;
	stdio_redirector* that = reinterpret_cast<stdio_redirector*>(lpUser);

	handles[0] = that->_hChildProcess;
	handles[1] = that->_hEventExit;

	for(;;) {
		r = that->redirect_stdout();
		if(r < 0) break;

		DWORD dwR = ::WaitForMultipleObjects(2, handles, FALSE, that->_dwWaitInterval);
		if(dwR == WAIT_OBJECT_0 + 0) {
			r = that->redirect_stdout();
			if(r > 0) r = 0;
			break;
		}
		else if(dwR == WAIT_OBJECT_0 + 1){
			r = 1;
			break;
		}

		//static int i = 0;
		//printf("looping...%d\n", ++i);
	}

	that->close();
	return r;
}

void stdio_redirector::write_stdout(LPCTSTR pszOutput)
{
	::printf(pszOutput);
}

void stdio_redirector::close()
{

}


