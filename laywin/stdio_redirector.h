#ifndef __stdio_redirector__
#define __stdio_redirector__

#pragma once

#include <windows.h>

class stdio_redirector {
public:
	stdio_redirector();
	virtual ~stdio_redirector();

private:
	HANDLE _hThread;
	HANDLE _hEventExit;
	DWORD _dwThreadId;
	DWORD _dwWaitInterval;

protected:
	HANDLE _hStdinWrite;
	HANDLE _hStdoutRead;
	HANDLE _hChildProcess;

	BOOL launch_child(LPCTSTR pszCmdLine,
		HANDLE hStdOut, HANDLE hStdin, HANDLE hStdErr);
	int redirect_stdout();
	void destroy_handle(HANDLE& h);

	static DWORD WINAPI thread_output(LPVOID lpUser);

protected:
	virtual void write_stdout(LPCTSTR pszOutput);

public:
	bool open(LPCTSTR pszCmdLine);
	void close();
	bool print(LPCTSTR fmt, ...);
	void set_interval(DWORD dwInterval);
};

#endif //!__stdio_redirector__
