#if !defined(AFX_UPDATER_H__5523EEA1_4031_4830_B045_AF24E7D88F91__INCLUDED_)
#define AFX_UPDATER_H__5523EEA1_4031_4830_B045_AF24E7D88F91__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

typedef struct tagUPDATEITEM
{
	TCHAR szFileName[MAX_PATH];
	TCHAR szLocalMD5[33];
	TCHAR szServerMD5[33];
	DWORD dwSize;
	BOOL  bNeedUpdate;
} UPDATEITEM, *LPUPDATEITEM;

class CUpdaterApp : public CWinApp
{
public:
	CUpdaterApp();
	virtual ~CUpdaterApp();

protected:
	DWORD		m_dwProcessId;
	HWND		m_hWindowCtrl;
	TCHAR		m_szServer[512];

public:
	CArray<LPUPDATEITEM, LPUPDATEITEM> m_arrUpdate;

public:
	LPCTSTR GetServerAdress() const { return m_szServer; }

public:
	BOOL LoadPngFromRes(CImage& image, LPCTSTR pszResName);

protected:
	BOOL _InitCmdline(int argc, LPTSTR* args);
	BOOL _InitFileList();
	BOOL _CheckNeedUpdate();

protected:
	static DWORD WINAPI _CheckFileMD5(LPVOID lpParameter);

public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

extern CUpdaterApp theApp;

#endif // !defined(AFX_UPDATER_H__5523EEA1_4031_4830_B045_AF24E7D88F91__INCLUDED_)
