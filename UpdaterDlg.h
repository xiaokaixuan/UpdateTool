#if !defined(AFX_UPDATERDLG_H__18191834_5682_4615_91CA_0D35BA2A570C__INCLUDED_)
#define AFX_UPDATERDLG_H__18191834_5682_4615_91CA_0D35BA2A570C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SingleProcess.h"
#include "LayeredWnd.h"

#define UM_OVERDOWNLOAD (WM_USER+201)
#define UM_UPDATEGRESS	(WM_USER+202)

typedef struct tagGRESSINFO
{
	TCHAR szName[MAX_PATH];
	int	  nPos;
	float fSpeed;
} GRESSINFO, *LPGRESSINFO;

class CUpdaterDlg : public CDialog
{
public:
	CUpdaterDlg(CWnd* pParent = NULL);
	virtual ~CUpdaterDlg();

public:
	enum { IDD = IDD_UPDATER_DIALOG };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	HANDLE			m_hEveQuit;
	CWinThread*		m_pThread;

protected:
	GRESSINFO		m_gresInfo;

protected:
	CSingleProcess  m_spObject;
	CLayeredWnd		m_wndLayered;

protected:
	void _Proc();
	void _DownloadFile(LPUPDATEITEM lpUpdateItem, PBOOL pIsBreak);
	static UINT _DownloadThread(LPVOID pParam);

protected:
	CImage			m_imgBack;
	CImage			m_imgGres;
	CImage			m_imgPoint;

protected:
	HICON m_hIcon;

	DECLARE_MESSAGE_MAP()

	virtual void OnOK();
	virtual void OnCancel();

public:
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg LRESULT OnUpdateGress(WPARAM wParam, LPARAM lParam);
};

#endif // !defined(AFX_UPDATERDLG_H__18191834_5682_4615_91CA_0D35BA2A570C__INCLUDED_)
