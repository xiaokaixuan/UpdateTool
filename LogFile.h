#if !defined(_LOGFILE_H__261C9DDC_AB17_4781_B87F_4B82DD38DD13__INCLUDED)
#define _LOGFILE_H__261C9DDC_AB17_4781_B87F_4B82DD38DD13__INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <tchar.h>
#include <stdio.h>
#include <stdarg.h>
#include <clocale>
#include <cstdio>
#include <shlwapi.h>

#if !defined(ASSERT)
#include <cassert>
#define ASSERT assert
#endif // !defined(ASSERT)

#pragma comment(lib, "shlwapi.lib")

#pragma warning(disable: 4996)

/////////////////////////////////////////////////////////////////////////////////////
/*
CLogFile class
	Recording log is this class's work.
*/
class CLogFile
{
public:
	// Init CLogFile instance logger.
	static CLogFile* InitLogger(LPCTSTR lpLogName = NULL);

	// Record log to log file.
	static void Log(LPCSTR lpFile, DWORD dwLine, LPCTSTR lpFormat, ...);

public:
	// Write log to log file method.
	void Write(LPCSTR lpFile, DWORD dwLine, LPCTSTR lpMessage);

public:
	// Destructor.
	virtual ~CLogFile();

public:
	class FOTracer
	{
	private:
		LPCSTR m_lpFile;
		DWORD  m_dwLine;
	public:
		FOTracer(LPCSTR lpFile, DWORD dwLine): m_lpFile(lpFile), m_dwLine(dwLine){}
		
		void operator()(LPCTSTR lpFormat, ...);
	};

	friend class FOTracer;

private:
	CLogFile(LPCTSTR lpLogName);
	CLogFile(const CLogFile &other);
	CLogFile& operator = (const CLogFile &);

private:
	static CLogFile* m_pMe;

private:
	FILE* m_pFile;
	CRITICAL_SECTION m_hWrCrtSec;
};

__declspec(selectany) CLogFile* CLogFile::m_pMe = NULL;


#define InitializeLogger		CLogFile::InitLogger
#define LOG						CLogFile::FOTracer(__FILE__, __LINE__)

/////////////////////////////////////////////////////////////////////////////////////


/************************************************************************************
Constructor.
*/
inline CLogFile::CLogFile(LPCTSTR lpLogName)
{
#if defined(LOG_CONSOLE)
#if !defined(_CONSOLE)
	AllocConsole();
	_tfreopen(_T("CONOUT$"), _T("w+"), stdout);
	
	SetConsoleTitle(_T("Log Recorder"));

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo = {0};
	GetConsoleScreenBufferInfo(hConsole, &csbiInfo);

	SMALL_RECT rect = csbiInfo.srWindow;
	COORD coord = {200, 600};
	rect.Right = 599;

	SetConsoleScreenBufferSize(hConsole, coord);
	SetConsoleWindowInfo(hConsole, FALSE, &rect);
	CloseHandle(hConsole);
#endif // !defined(_CONSOLE)
#else
#if !defined(_DEBUG)
	m_pFile = _tfsopen(lpLogName, _T("a+"), 0x20);

	int nNum(1);
	while (!m_pFile)
	{
		TCHAR szExtName[44] = {0};
		_stprintf(szExtName, _T(".%d.log"), nNum++);
		TCHAR szLogName[MAX_PATH] = {0};
		lstrcpyn(szLogName, lpLogName, MAX_PATH);
		PathRenameExtension(szLogName, szExtName);
		m_pFile = _tfsopen(szLogName, _T("a+"), 0x20);
	}
#endif // !defined(_DEBUG)
#endif // defined(LOG_CONSOLE)

	InitializeCriticalSection(&m_hWrCrtSec);
}

/************************************************************************************
Copy constructor.
*/
inline CLogFile::CLogFile(const CLogFile &other)
{
	*this = other;
}

/************************************************************************************
Operator assignment overload.
*/
inline CLogFile& CLogFile::operator = (const CLogFile&)
{
	return *this;
}

/************************************************************************************
Constructor.
*/
inline CLogFile::~CLogFile()
{
	DeleteCriticalSection(&m_hWrCrtSec);

#if defined(LOG_CONSOLE)
#if !defined(_CONSOLE)
	FreeConsole();
#endif // !defined(_CONSOLE)
#else
#if !defined(_DEBUG)
	if (m_pFile)
	{
		fclose(m_pFile);
	}
#endif // !defined(_DEBUG)
#endif // defined(LOG_CONSOLE)
}

/************************************************************************************
Init CLogFile instance logger.
*/
inline CLogFile* CLogFile::InitLogger(LPCTSTR lpLogName/* = NULL*/)
{
	if (!m_pMe)
	{
		_tsetlocale(LC_ALL, _T("chs"));
		TCHAR szLogName[MAX_PATH] = {0};
		if (lpLogName)
		{
			lstrcpyn(szLogName, lpLogName, MAX_PATH);
		}
		else
		{
			HMODULE hModule(NULL);
#if defined(GetModuleHandleEx)
			LPCTSTR pszModuleName = (LPCTSTR)InitLogger;
			GetModuleHandleEx(0x00000004, pszModuleName, &hModule);
#endif // defined(GetModuleHandleEx)
			GetModuleFileName(hModule, szLogName, MAX_PATH);
			PathRenameExtension(szLogName, _T(".log"));
		}
		static CLogFile Me(szLogName);
		m_pMe = &Me;
	}
	return m_pMe;
}

/************************************************************************************
Write log to log file method.
*/
inline void CLogFile::Write(LPCSTR lpFile, DWORD dwLine, LPCTSTR lpMessage)
{
	LPCSTR lpFileName = PathFindFileNameA(lpFile);

	TCHAR szFileName[MAX_PATH] = {0};
#if defined(_UNICODE) || defined(UNICODE)
	mbstowcs(szFileName, lpFileName, MAX_PATH);
#else
	lstrcpyn(szFileName, lpFileName, MAX_PATH);
#endif // defined(_UNICODE) || defined(UNICODE)

	TCHAR szDateTime[256] = {0};
	TCHAR szLogMessage[4096] = {0};

	SYSTEMTIME st = {0};
	GetLocalTime(&st);

	_stprintf(szDateTime, _T("%04d%02d%02d %02d:%02d:%02d:%03d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	_stprintf(szLogMessage, _T("[%s][%d][%d][%s:%d]%s\n"), szDateTime, GetCurrentProcessId(), GetCurrentThreadId(), szFileName, dwLine, lpMessage);

	EnterCriticalSection(&m_hWrCrtSec);

#if defined(LOG_CONSOLE)
	_tprintf(szLogMessage);
#else

#ifdef _DEBUG
	OutputDebugString(szLogMessage);
#else
	_fputts(szLogMessage, m_pFile);
	fflush(m_pFile);
#endif // _DEBUG

#endif // !defined(LOG_CONSOLE)

	LeaveCriticalSection(&m_hWrCrtSec);
}

/************************************************************************************
Record log to log file.
*/
inline void CLogFile::Log(LPCSTR lpFile, DWORD dwLine, LPCTSTR lpFormat, ...)
{
	ASSERT(NULL != m_pMe);

	if (m_pMe)
	{
		TCHAR szMessage[3030] = {0};
		
		va_list args;
		va_start(args, lpFormat);
		
		_vstprintf(szMessage, lpFormat, args);
		
		va_end(args);
		
		m_pMe->Write(lpFile, dwLine, szMessage);
	}
}

inline void CLogFile::FOTracer::operator()(LPCTSTR lpFormat, ...)
{
	ASSERT(NULL != CLogFile::m_pMe);

	if (CLogFile::m_pMe)
	{
		TCHAR szMessage[3030] = {0};
		
		va_list args;
		va_start(args, lpFormat);
		
		_vstprintf(szMessage, lpFormat, args);
		
		va_end(args);
		
		CLogFile::m_pMe->Write(m_lpFile, m_dwLine, szMessage);
	}
}

#endif // !defined(_LOGFILE_H__261C9DDC_AB17_4781_B87F_4B82DD38DD13__INCLUDED)

