#include "StdAfx.h"
#include "Timer.h"
#include "Message.h"
#include "Updater.h"
#include "CmdLine.h"
#include "SingleProcess.h"
#include "ConfigFile.h"
#include "ThreadPool.h"
#include "md5.h"
#include <Psapi.h>
#include "UpdaterDlg.h"

// 消息映射
BEGIN_MESSAGE_MAP(CUpdaterApp, CWinApp)
END_MESSAGE_MAP()

// 构造函数
CUpdaterApp::CUpdaterApp()
: m_dwProcessId(0)
, m_hWindowCtrl(0)
{
	ZeroMemory(m_szServer, sizeof(m_szServer));
}

// 析构函数
CUpdaterApp::~CUpdaterApp()
{
	int nCount = m_arrUpdate.GetSize();
	if (nCount > 0)
	{
		for (int i(0); i < nCount; ++i)
		{
			delete m_arrUpdate.GetAt(i);
		}
		m_arrUpdate.RemoveAll();
	}
}

// 加载PNG图片资源
BOOL CUpdaterApp::LoadPngFromRes(CImage& image, LPCTSTR pszResName)
{
	BOOL bReturn(FALSE);
	HINSTANCE hInst = AfxGetInstanceHandle();
	HRSRC hRsrc = ::FindResource(hInst, pszResName, _T("PNG"));
	if (hRsrc)
	{
		DWORD len = SizeofResource(hInst, hRsrc);
		BYTE* lpRsrc = (BYTE*)LoadResource(hInst, hRsrc);
		if (lpRsrc)
		{
			HGLOBAL hMem = GlobalAlloc(GMEM_FIXED, len);
			BYTE* pmem = (BYTE*)GlobalLock(hMem);
			memcpy(pmem, lpRsrc, len);
			IStream* pstm = NULL;
			if (S_OK == CreateStreamOnHGlobal(hMem, FALSE, &pstm))
			{
				image.Load(pstm);
				bReturn = TRUE;
			}
		}
	}
	return bReturn;
}

CUpdaterApp    theApp;
CSingleProcess thePid(_T("Update-PID"));

// 应用程序初始化
BOOL CUpdaterApp::InitInstance()
{
	// 确保单进程
	if (thePid.IsExist()) return FALSE;

	// 日志记录
	InitializeLogger();

	// 命令行参数
	if (_InitCmdline(__argc, __targv))
	{
		// 是否开启更新
		CConfigFile conf(_T("Setting.ini"));
		CString strAllow;
		conf.GetKeyString(_T("Update"), _T("Allow"), _T("true"), strAllow);
		if (!strAllow.CompareNoCase(_T("true")))
		{
			// 服务器
			conf.GetKeyString(_T("Update"), _T("ServerAddress"), _T(""), m_szServer, 511);
			int nLen = lstrlen(m_szServer);
			if (nLen > 0)
			{
				if (m_szServer[nLen - 1] == _T('/'))
				{
					m_szServer[nLen - 1] = _T('\0');
				}

				// 初始化下载列表
				if (_InitFileList())
				{
					// 等待进程结束，需主进程自己结束
					HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, m_dwProcessId);
					TCHAR szExeFile[512] = {0};
					GetModuleFileNameEx(hProcess, NULL, szExeFile, 511);

					// 检查是否需要更新
					if (_CheckNeedUpdate())
					{
						WaitForSingleObject(hProcess, INFINITE);
						CloseHandle(hProcess);

						// 开始更新
						CUpdaterDlg dlg;
						dlg.DoModal();

						// 启动大厅
						ShellExecute(NULL, _T("Open"), szExeFile, NULL, NULL, SW_SHOW);
					}
					else
					{
						// 已是最新或者主进程未允许更新
						if (IsWindow(m_hWindowCtrl))
						{
							PostMessage(m_hWindowCtrl, UM_NONEEDUPDATE, 0, 0);
						}
					}
				}
				else
				{
					LOG(_T("初始化下载列表失败"));
					if (IsWindow(m_hWindowCtrl))
					{
						PostMessage(m_hWindowCtrl, UM_CHECKFILEERROR, 0, 0);
					}
				}
			}
			else
			{
				LOG(_T("未配置更新地址！"));
				if (IsWindow(m_hWindowCtrl))
				{
					PostMessage(m_hWindowCtrl, UM_DISABLEUPDATE, 0, 0);
				}
			}
		}
		else
		{
			LOG(_T("未开启更新！"));
			if (IsWindow(m_hWindowCtrl))
			{
				PostMessage(m_hWindowCtrl, UM_DISABLEUPDATE, 0, 0);
			}
		}
	}
	else
	{
		LOG(_T("初始化命令行失败！"));
	}

	return FALSE;
}

// 初始化命令行参数
BOOL CUpdaterApp::_InitCmdline(int argc, LPTSTR* args)
{
	BOOL bReturn(FALSE);
	CCmdLine cmder;
	if (cmder.SplitLine(__argc, __targv) >= 2)
	{
		if (cmder.HasSwitch(_T("-PID")) && cmder.HasSwitch(_T("-H")))
		{
			m_dwProcessId = (DWORD)_ttoi(cmder.GetSafeArgument(_T("-PID"), 0, _T("0")));
			m_hWindowCtrl = (HWND)_ttoi(cmder.GetSafeArgument(_T("-H"), 0, _T("0")));
			bReturn = TRUE;
		}
		else
		{
			LOG(_T("命令行参数错误！"));
		}
	}
	else
	{
		LOG(_T("未指定命令行参数"));
	}
	return bReturn;
}

// 切割字符串
std::vector<CString> Split(LPCTSTR pszStrLine, LPCTSTR pszDelim)
{
	std::vector<CString> vecReturn;
	CString strLine(pszStrLine), strDelim(pszDelim);
	int nPos = strLine.Find(strDelim);
	while (-1 != nPos)
	{
		if (nPos > 0)
		{
			vecReturn.push_back(strLine.Left(nPos));
		}
		strLine.Delete(0, nPos + strDelim.GetLength());
		nPos = strLine.Find(strDelim);
	}
	if (!strLine.IsEmpty())
	{
		vecReturn.push_back(strLine);
	}
	return vecReturn;
}

// 初始化更新列表
BOOL CUpdaterApp::_InitFileList()
{
	BOOL bReturn(FALSE);

	CString strFileList(m_szServer);
	strFileList += _T("/Filelist.txt");

	CInternetSession sess;
	sess.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 5000);
	sess.SetOption(INTERNET_OPTION_SEND_TIMEOUT, 1000);
	sess.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 5000);
	sess.SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, 1000);
	sess.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 5000);
	sess.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 1);

	try
	{
		CHttpFile* pHttpFile = (CHttpFile*)sess.OpenURL(strFileList, 1, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_TRANSFER_ASCII, NULL, 0);
		if (pHttpFile)
		{
			DWORD dwErrCode(0);
			pHttpFile->QueryInfo(HTTP_QUERY_STATUS_CODE, dwErrCode);
			if (dwErrCode >= 200 && dwErrCode < 300)
			{
				CStringA strLineA;
				while (pHttpFile->ReadString((LPTSTR)strLineA.GetBuffer(512), 511))
				{
					strLineA.ReleaseBuffer(); // MFC-bug
					CString strLine(strLineA);
					strLine.TrimLeft();
					if (strLine.GetLength() > 0 && strLine[0] != _T('\\'))
					{
						continue;
					}
					std::vector<CString> vecStrs = Split(strLine, _T("^^^^^^"));
					if (vecStrs.size() >= 2)
					{
						LPUPDATEITEM pUpdateItem = new UPDATEITEM;
						ZeroMemory(pUpdateItem, sizeof(*pUpdateItem));
						lstrcpyn(pUpdateItem->szFileName, vecStrs[0], MAX_PATH);
						lstrcpyn(pUpdateItem->szServerMD5, vecStrs[1], 33);
						m_arrUpdate.Add(pUpdateItem);
					}
				}
				pHttpFile->Close();
				sess.Close();
				bReturn = TRUE;
			}
			else
			{
				LOG(_T("网站访问错误码：%d"), dwErrCode);
			}
		}
	}
	catch (...)
	{
		LOG(_T("下载列表异常！"));
	}

	return bReturn;
}

// 验证是否需要更新
BOOL CUpdaterApp::_CheckNeedUpdate()
{
	BOOL bReturn(FALSE);

	int nCount = m_arrUpdate.GetSize();
	if (nCount > 0)
	{
		// 启动线程池
		SYSTEM_INFO si = {0};
		GetSystemInfo(&si);
		CThreadPool *pThreadPool = new CThreadPool(32);
		for (int i(0); i < nCount; ++i)
		{
			pThreadPool->Run(_CheckFileMD5, m_arrUpdate.GetAt(i));
		}
		pThreadPool->Wait(INFINITE);
		delete pThreadPool;

		// 检查是否存在更新
		BOOL bExistUpdate(FALSE);
		for (int j(0); j < nCount; ++j)
		{
			if (m_arrUpdate.GetAt(j)->bNeedUpdate)
			{
				bExistUpdate = TRUE;
				break;
			}
		}
		
		if (bExistUpdate)
		{
			// 询问控制窗口
			if (IsWindow(m_hWindowCtrl))
			{
				if (0 == SendMessage(m_hWindowCtrl, UM_IFNEEDUPDATE, 0, 0))
				{
					// 返回值0允许更新
					bReturn = TRUE;
				}
			}
		}
	}
	else
	{
		LOG(_T("更新列表为空！"));
	}

	return bReturn;
}

// 检查文件MD5值
DWORD WINAPI CUpdaterApp::_CheckFileMD5(LPVOID lpParameter)
{
	LPUPDATEITEM pItem = (LPUPDATEITEM)lpParameter;
	TCHAR szFileName[MAX_PATH] = {0};
	GetModuleFileName(NULL, szFileName, MAX_PATH - 1);
	PathRemoveFileSpec(szFileName);
	PathAppend(szFileName, pItem->szFileName);
	
	HANDLE hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE != hFile)
	{
		HANDLE hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
		if (hMap)
		{
			DWORD dwSize = GetFileSize(hFile, NULL);
			LPVOID lpBuff = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
			MD5 md5(lpBuff, dwSize);
#if defined(_UNICODE) || defined(UNICODE)
			mbstowcs(pItem->szLocalMD5, md5.toString().c_str(), 32);
#else
			lstrcpyn(pItem->szLocalMD5, md5.toString().c_str(), 33);
#endif // defined(_UNICODE) || defined(UNICODE)
			if (lstrcmpi(pItem->szLocalMD5, pItem->szServerMD5))
			{
				pItem->bNeedUpdate = TRUE;
			}
			UnmapViewOfFile(lpBuff);
			CloseHandle(hMap);
		}
		CloseHandle(hFile);
	}
	else
	{
		if (!PathFileExists(szFileName))
		{
			// 文件不存在，标记更新！
			pItem->bNeedUpdate = TRUE;
		}
	}
	return 0;
}
