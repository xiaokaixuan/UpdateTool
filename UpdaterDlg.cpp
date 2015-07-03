#include "StdAfx.h"
#include "Updater.h"
#include "Timer.h"
#include "SingleProcess.h"
#include "UtilityFunc.h"
#include "UpdaterDlg.h"


CUpdaterDlg::CUpdaterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUpdaterDlg::IDD, pParent)
	, m_pThread(NULL)
{
	ZeroMemory(&m_gresInfo, sizeof(m_gresInfo));

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	theApp.LoadPngFromRes(m_imgBack, _T("IMG_BACK"));
	theApp.LoadPngFromRes(m_imgGres, _T("IMG_GRES"));
	theApp.LoadPngFromRes(m_imgPoint, _T("IMG_PT"));
	CrossImage(m_imgGres);
	CrossImage(m_imgPoint);

	m_hEveQuit = CreateEvent(NULL, TRUE, FALSE, NULL);
}

CUpdaterDlg::~CUpdaterDlg()
{
	CloseHandle(m_hEveQuit);
}

void CUpdaterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CUpdaterDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SHOWWINDOW()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_NCHITTEST()
	ON_MESSAGE(UM_UPDATEGRESS, OnUpdateGress)
END_MESSAGE_MAP()

BOOL CUpdaterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	m_spObject.SetSingleFlag(_T("后宫OL"));
	m_spObject.SetMainWnd(GetSafeHwnd());

	int cx = m_imgBack.GetWidth();
	int cy = m_imgBack.GetHeight();

	int d = m_imgBack.GetBPP();

	CRect rcUnLayered;
	rcUnLayered.top    = 35;
	rcUnLayered.left   = 30;
	rcUnLayered.right  = cx - 10;
	rcUnLayered.bottom = cy - 10;

	CRgn RgnWindow;
	RgnWindow.CreateRoundRectRgn(rcUnLayered.left, rcUnLayered.top, rcUnLayered.right + 1, rcUnLayered.bottom + 1, 5, 5);

	SetWindowRgn(RgnWindow, FALSE);

	m_wndLayered.CreateLayered(this, CRect(0, 0, cx, cy));
	m_wndLayered.InitLayeredArea(m_imgBack, 255, rcUnLayered, CPoint(5, 5), FALSE);

	CenterWindow();
	SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	
	m_pThread = AfxBeginThread(_DownloadThread, this);
	m_pThread->m_bAutoDelete = FALSE;
	
	return TRUE;
}

UINT CUpdaterDlg::_DownloadThread(LPVOID pParam)
{
	CUpdaterDlg* pThis = (CUpdaterDlg*)pParam;
	pThis->_Proc();

	return 0;
}

void CUpdaterDlg::_DownloadFile(LPUPDATEITEM lpUpdateItem, PBOOL pIsBreak)
{
	// 数据校验
	if (!lpUpdateItem || !lpUpdateItem->bNeedUpdate) return;
	if (pIsBreak)
	{
		*pIsBreak = FALSE;
	}

	// 本地路径
	TCHAR szFolder[512] = {0};
	GetModuleFileName(NULL, szFolder, 511);
	PathRemoveFileSpec(szFolder);

	// 服务器地址
	CString strServer = theApp.GetServerAdress();

	CInternetSession sess;
	sess.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 7000);
	sess.SetOption(INTERNET_OPTION_SEND_TIMEOUT, 1000);
	sess.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 5000);
	sess.SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, 1000);
	sess.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 5000);
	sess.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 5);

	// 构造URL
	CString strUrl(strServer);
	strUrl += lpUpdateItem->szFileName;
	strUrl.Replace(_T('\\'), _T('/'));

	// 临时文件
	CString strDownload(szFolder);
	PathAppend(strDownload.GetBuffer(512), _T("UpdateFolder"));
	strDownload.ReleaseBuffer();
	if (!PathFileExists(strDownload))
	{
		CreateDirectory(strDownload, NULL);
	}
	LPTSTR pszDownload = strDownload.GetBuffer(512);
	PathAppend(pszDownload, PathFindFileName(lpUpdateItem->szFileName));
	PathRenameExtension(pszDownload, _T(".download"));
	strDownload.ReleaseBuffer();

	try
	{
		CHttpFile *pHttpFile = (CHttpFile*)sess.OpenURL(strUrl, 1, INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_DONT_CACHE);
		if (pHttpFile)
		{
			// HTTP状态码
			DWORD dwStatus(0);
			pHttpFile->QueryInfo(HTTP_QUERY_STATUS_CODE, dwStatus);
			if (dwStatus >=200 && dwStatus < 300)
			{
				int nPos = 0;			// 进度条
				DWORD dwCurLen(0);
				DWORD dwLength(0);		// 总大小
				pHttpFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, dwLength);
				if (dwLength > 0)
				{
					CFile out(strDownload, CFile::modeCreate | CFile::modeWrite);
					CTimer time;
					BYTE cbBuffer[1024] = {0};
					UINT uReaded = pHttpFile->Read(cbBuffer, sizeof(cbBuffer));
					while (uReaded > 0)
					{
						if (WaitForSingleObject(m_hEveQuit, 0) == WAIT_OBJECT_0)
						{
							if (pIsBreak) *pIsBreak = TRUE;
							break;
						}
						out.Write(cbBuffer, uReaded);
						dwCurLen += uReaded;
						if (dwCurLen*100/dwLength > (DWORD)nPos)
						{
							nPos = dwCurLen*100/dwLength;
							LPGRESSINFO pGressInfo = new GRESSINFO;
							pGressInfo->nPos = nPos;
							pGressInfo->fSpeed = float(dwCurLen/time.Current()/1024);
							lstrcpyn(pGressInfo->szName, PathFindFileName(lpUpdateItem->szFileName), MAX_PATH);
							PostMessage(UM_UPDATEGRESS, 0, LPARAM(pGressInfo));
						}
						uReaded = pHttpFile->Read(cbBuffer, sizeof(cbBuffer));
					}
					out.Close();

					if (!(*pIsBreak))
					{
						CString strLocal(szFolder);
						PathAppend(strLocal.GetBuffer(512), lpUpdateItem->szFileName);
						strLocal.ReleaseBuffer();

						// 判断自我更新
						TCHAR szExeFile[512] = {0};
						GetModuleFileName(NULL, szExeFile, 511);
						if (0 == strLocal.CompareNoCase(szExeFile))
						{
							CString strBackup(szExeFile);
							PathRenameExtension(strBackup.GetBuffer(512), _T(".tmp"));
							strBackup.ReleaseBuffer();
							if (PathFileExists(strBackup))
							{
								if (DeleteFile(strBackup))
								{
									MoveFile(szExeFile, strBackup);
								}
							}
							else
							{
								MoveFile(szExeFile, strBackup);
							}

							TCHAR szTemp[512] = {0};
							GetTempPath(511, szTemp);
							PathAppend(szTemp, PathFindFileName(strBackup));
							if (PathFileExists(szTemp))
							{
								if (DeleteFile(szTemp))
								{
									MoveFile(strBackup, szTemp);
								}
							}
							else
							{
								MoveFile(strBackup, szTemp);
							}
						}
						SHMoveFile(strDownload, strLocal);
					}
				}
			}
			else
			{
				LOG(_T("HTTP错误码:%d %s"), dwStatus, lpUpdateItem->szFileName);
			}
			pHttpFile->Close();
		}
	}
	catch(...)
	{
		LOG(_T("%s HTTP请求异常！"), lpUpdateItem->szFileName);
	}
	sess.Close();
}

void CUpdaterDlg::_Proc()
{	
	CArray<LPUPDATEITEM, LPUPDATEITEM>& arrUpdate = theApp.m_arrUpdate;
	int nItemCount = arrUpdate.GetSize();

	for (int i(0); WaitForSingleObject(m_hEveQuit, 0) != WAIT_OBJECT_0 && i < nItemCount; ++i)
	{
		LPUPDATEITEM pItem = arrUpdate.GetAt(i);
		if (pItem->bNeedUpdate)
		{
			BOOL bIsBreak = FALSE;
			_DownloadFile(pItem, &bIsBreak);
			if (bIsBreak)
			{
				break;
			}
		}
	}
	PostMessage(WM_COMMAND, IDCANCEL, 0);
}

void CUpdaterDlg::OnOK()
{

}

void CUpdaterDlg::OnCancel()
{
	SetEvent(m_hEveQuit);
	DWORD dwReturn = WaitForSingleObject(m_pThread->m_hThread, 200);
	if (dwReturn != WAIT_TIMEOUT)
	{
		delete m_pThread;
		CDialog::OnCancel();
	}
}

void CUpdaterDlg::OnDestroy()
{
	m_spObject.DeleteObject();
	CDialog::OnDestroy();
}

BOOL CUpdaterDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	CImage ImageBuffer;
	ImageBuffer.CreateEx(rcClient.Width(), rcClient.Height(), 32, BI_RGB, NULL, CImage::createAlphaChannel);

	CImageDC BufferDC(ImageBuffer);
	CDC * pBufferDC = CDC::FromHandle(BufferDC);

	pBufferDC->SetBkMode(TRANSPARENT);

	m_imgBack.Draw(pBufferDC->GetSafeHdc(), 0, 0);

	CSize SizeStatusLine;
	CSize SizeStatusPoint;
	SizeStatusLine.SetSize(m_imgGres.GetWidth(), m_imgGres.GetHeight()/2);
	SizeStatusPoint.SetSize(m_imgPoint.GetWidth(), m_imgPoint.GetHeight());

	m_imgGres.Draw(pBufferDC->GetSafeHdc(), 110, 45, SizeStatusLine.cx, SizeStatusLine.cy, 0, 0, SizeStatusLine.cx, SizeStatusLine.cy);
	if (m_gresInfo.nPos > 0)
	{
		m_imgGres.Draw(pBufferDC->GetSafeHdc(), 110, 45, SizeStatusLine.cx*m_gresInfo.nPos/100L, SizeStatusLine.cy, 0, SizeStatusLine.cy, SizeStatusLine.cx*m_gresInfo.nPos/100L, SizeStatusLine.cy);
	}

	INT nXPoint=SizeStatusLine.cx*m_gresInfo.nPos/100L;
	m_imgPoint.Draw(pBufferDC->GetSafeHdc(), 105+nXPoint-SizeStatusPoint.cx/2, 45+(SizeStatusLine.cy-SizeStatusPoint.cy)/2);

	pBufferDC->SetTextColor(RGB(0, 0, 0));
	pBufferDC->SelectObject(GetFont());
	CString strOut;
	strOut.Format(_T("下载%s中... %.2fKB/s."), m_gresInfo.szName, m_gresInfo.fSpeed);
	pBufferDC->TextOut(115, 72, strOut);

	pDC->BitBlt(0, 0, rcClient.Width(), rcClient.Height(), pBufferDC, 0, 0, SRCCOPY);

	return TRUE;
}

void CUpdaterDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	if (m_wndLayered.m_hWnd!=NULL)
	{
		m_wndLayered.ShowWindow((bShow) ? SW_SHOW:SW_HIDE);
	}
}

void CUpdaterDlg::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CDialog::OnWindowPosChanged(lpwndpos);

	if ((m_wndLayered.m_hWnd!=NULL)&&(lpwndpos->cx>=0)&&(lpwndpos->cy>0))
	{
		m_wndLayered.SetWindowPos(NULL, lpwndpos->x, lpwndpos->y, lpwndpos->cx, lpwndpos->cy, SWP_NOZORDER);
	}
}

LRESULT CUpdaterDlg::OnNcHitTest(CPoint point)
{
	return HTCAPTION;
}

LRESULT CUpdaterDlg::OnUpdateGress(WPARAM wParam, LPARAM lParam)
{
	LPGRESSINFO pGressInfo = LPGRESSINFO(lParam);
	CopyMemory(&m_gresInfo, pGressInfo, sizeof(m_gresInfo));
	delete pGressInfo;

	Invalidate();

	return 0;
}
