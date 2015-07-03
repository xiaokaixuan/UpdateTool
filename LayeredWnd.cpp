#include "StdAfx.h"
#include "LayeredWnd.h"


//构造函数
CLayeredWnd::CLayeredWnd()
{
	//设置变量
	m_pWndControl = NULL;
}

//析构函数
CLayeredWnd::~CLayeredWnd()
{
}

//创建窗口
VOID CLayeredWnd::CreateLayered(CWnd* pWndControl, const CRect& rcWindow)
{
	//效验参数
	ASSERT(pWndControl);

	//设置变量
	m_pWndControl = pWndControl;

	//创建窗口
	CreateEx(WS_EX_LAYERED, AfxRegisterWndClass(0), _T(""), 0, rcWindow, pWndControl, 0);

	return;
}

BEGIN_MESSAGE_MAP(CLayeredWnd, CWnd)
	ON_WM_CLOSE()
	ON_WM_SETFOCUS()
	ON_WM_NCHITTEST()
	ON_WM_WINDOWPOSCHANGED()
END_MESSAGE_MAP()

//子窗口信息
struct tagEnumChildInfo
{
	CWnd* pWndLayered;
	CWnd* pWndControl;
	CRgn* pRegionResult;
};

//设置区域
VOID CLayeredWnd::InitLayeredArea(CDC* pDCImage, BYTE cbAlpha, const CRect & rcUnLayered, const CPoint & PointRound, BOOL bUnLayeredChild)
{
	//效验参数
	ASSERT((pDCImage) && (pDCImage->m_hDC));
	if ((pDCImage == NULL) || (pDCImage->m_hDC == NULL)) return;

	//变量定义
	BITMAP Bitmap = {0};
	ZeroMemory(&Bitmap, sizeof(Bitmap));

	//获取图像
	CBitmap* pBitmap = pDCImage->GetCurrentBitmap();
	if (pBitmap) pBitmap->GetBitmap(&Bitmap);

	//获取大小
	CSize SizeImage;
	SizeImage.SetSize(Bitmap.bmWidth, Bitmap.bmHeight);

	//效验大小
	ASSERT((SizeImage.cx > 0) && (SizeImage.cy > 0));
	if ((SizeImage.cx == 0) || (SizeImage.cy == 0)) return;

	//变量定义
	BLENDFUNCTION BlendFunction = {0};
	ZeroMemory(&BlendFunction, sizeof(BlendFunction));

	//设置参数
	BlendFunction.BlendOp = 0;
	BlendFunction.BlendFlags = 0;
	BlendFunction.AlphaFormat = AC_SRC_ALPHA;
	BlendFunction.SourceConstantAlpha = cbAlpha;

	//设置分层
	CPoint ImagePoint(0, 0);
	CClientDC ClientDC(this);
	UpdateLayeredWindow(&ClientDC, NULL, &SizeImage, pDCImage, &ImagePoint, 0L, &BlendFunction, ULW_ALPHA);

	//创建区域
	CRgn RegionResult;
	RegionResult.CreateRectRgn(0, 0, SizeImage.cx, SizeImage.cy);

	//窗口排除
	if (bUnLayeredChild)
	{
		//变量定义
		tagEnumChildInfo EnumChildInfo = {0};
		ZeroMemory(&EnumChildInfo, sizeof(EnumChildInfo));

		//设置变量
		EnumChildInfo.pWndLayered = this;
		EnumChildInfo.pWndControl = m_pWndControl;
		EnumChildInfo.pRegionResult = &RegionResult;

		//枚举窗口
		ASSERT(m_pWndControl->GetSafeHwnd());
		EnumChildWindows(m_pWndControl->m_hWnd, EnumChildProc, (LPARAM)&EnumChildInfo);
	}

	//区域排除
	if (!rcUnLayered.IsRectEmpty())
	{
		//创建区域
		CRgn RegionUnLayered;
		RegionUnLayered.CreateRoundRectRgn(rcUnLayered.left, rcUnLayered.top, rcUnLayered.right + 1, rcUnLayered.bottom + 1, PointRound.x, PointRound.y);

		//合并区域
		RegionResult.CombineRgn(&RegionResult, &RegionUnLayered, RGN_DIFF);
	}

	//设置区域
	SetWindowRgn(RegionResult, TRUE);
}

//设置区域
VOID CLayeredWnd::InitLayeredArea(CImage& Image, BYTE cbAlpha,  const CRect& rcUnLayered, const CPoint& PointRound, BOOL bUnLayeredChild)
{
	//创建缓冲
	CImage ImageBuffer;
	ImageBuffer.Create(Image.GetWidth(), Image.GetHeight(), 32);

	//绘画界面
	CImageDC ImageDC(ImageBuffer);
	CDC* pBufferDC = CDC::FromHandle(ImageDC);

	//绘画界面
	ASSERT(pBufferDC);
	if (pBufferDC) Image.Draw(pBufferDC->GetSafeHdc(), 0, 0);

	//创建分层
	InitLayeredArea(pBufferDC, cbAlpha, rcUnLayered, PointRound, bUnLayeredChild);

	return;
}

//关闭消息
VOID CLayeredWnd::OnClose()
{
	//投递消息
	if (m_pWndControl)
	{
		m_pWndControl->PostMessage(WM_CLOSE);
	}
}

//焦点消息
VOID CLayeredWnd::OnSetFocus(CWnd* pOldWnd)
{
	//设置焦点
	if (m_pWndControl)
	{
		m_pWndControl->SetFocus();
	}
}

//点击测试
LRESULT CLayeredWnd::OnNcHitTest(CPoint point)
{
	return HTCAPTION;
}

//位置改变
void CLayeredWnd::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CWnd::OnWindowPosChanged(lpwndpos);
	m_pWndControl->MoveWindow(lpwndpos->x, lpwndpos->y, lpwndpos->cx, lpwndpos->cy);
}

//枚举函数
BOOL CALLBACK CLayeredWnd::EnumChildProc(HWND hWndChild, LPARAM lParam)
{
	//获取位置
	CRect rcWindow;
	::GetWindowRect(hWndChild, &rcWindow);

	//创建区域
	if ((rcWindow.Width() > 0) && (rcWindow.Height() > 0))
	{
		//变量定义
		ASSERT(lParam);
		tagEnumChildInfo* pEnumChildInfo = (tagEnumChildInfo *)lParam;

		//窗口判断
		HWND hWndParent = ::GetParent(hWndChild);
		if (hWndParent != pEnumChildInfo->pWndControl->GetSafeHwnd())
		{
			return TRUE;
		}

		//转换位置
		ASSERT(pEnumChildInfo->pWndControl);
		pEnumChildInfo->pWndControl->ScreenToClient(&rcWindow);

		//创建区域
		CRgn RgnChild;
		RgnChild.CreateRectRgn(rcWindow.left, rcWindow.top, rcWindow.right, rcWindow.bottom);

		//合并区域
		ASSERT(pEnumChildInfo->pRegionResult);
		pEnumChildInfo->pRegionResult->CombineRgn(pEnumChildInfo->pRegionResult, &RgnChild, RGN_DIFF);
	}

	return TRUE;
}

