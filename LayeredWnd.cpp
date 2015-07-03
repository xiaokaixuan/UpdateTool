#include "StdAfx.h"
#include "LayeredWnd.h"


//���캯��
CLayeredWnd::CLayeredWnd()
{
	//���ñ���
	m_pWndControl = NULL;
}

//��������
CLayeredWnd::~CLayeredWnd()
{
}

//��������
VOID CLayeredWnd::CreateLayered(CWnd* pWndControl, const CRect& rcWindow)
{
	//Ч�����
	ASSERT(pWndControl);

	//���ñ���
	m_pWndControl = pWndControl;

	//��������
	CreateEx(WS_EX_LAYERED, AfxRegisterWndClass(0), _T(""), 0, rcWindow, pWndControl, 0);

	return;
}

BEGIN_MESSAGE_MAP(CLayeredWnd, CWnd)
	ON_WM_CLOSE()
	ON_WM_SETFOCUS()
	ON_WM_NCHITTEST()
	ON_WM_WINDOWPOSCHANGED()
END_MESSAGE_MAP()

//�Ӵ�����Ϣ
struct tagEnumChildInfo
{
	CWnd* pWndLayered;
	CWnd* pWndControl;
	CRgn* pRegionResult;
};

//��������
VOID CLayeredWnd::InitLayeredArea(CDC* pDCImage, BYTE cbAlpha, const CRect & rcUnLayered, const CPoint & PointRound, BOOL bUnLayeredChild)
{
	//Ч�����
	ASSERT((pDCImage) && (pDCImage->m_hDC));
	if ((pDCImage == NULL) || (pDCImage->m_hDC == NULL)) return;

	//��������
	BITMAP Bitmap = {0};
	ZeroMemory(&Bitmap, sizeof(Bitmap));

	//��ȡͼ��
	CBitmap* pBitmap = pDCImage->GetCurrentBitmap();
	if (pBitmap) pBitmap->GetBitmap(&Bitmap);

	//��ȡ��С
	CSize SizeImage;
	SizeImage.SetSize(Bitmap.bmWidth, Bitmap.bmHeight);

	//Ч���С
	ASSERT((SizeImage.cx > 0) && (SizeImage.cy > 0));
	if ((SizeImage.cx == 0) || (SizeImage.cy == 0)) return;

	//��������
	BLENDFUNCTION BlendFunction = {0};
	ZeroMemory(&BlendFunction, sizeof(BlendFunction));

	//���ò���
	BlendFunction.BlendOp = 0;
	BlendFunction.BlendFlags = 0;
	BlendFunction.AlphaFormat = AC_SRC_ALPHA;
	BlendFunction.SourceConstantAlpha = cbAlpha;

	//���÷ֲ�
	CPoint ImagePoint(0, 0);
	CClientDC ClientDC(this);
	UpdateLayeredWindow(&ClientDC, NULL, &SizeImage, pDCImage, &ImagePoint, 0L, &BlendFunction, ULW_ALPHA);

	//��������
	CRgn RegionResult;
	RegionResult.CreateRectRgn(0, 0, SizeImage.cx, SizeImage.cy);

	//�����ų�
	if (bUnLayeredChild)
	{
		//��������
		tagEnumChildInfo EnumChildInfo = {0};
		ZeroMemory(&EnumChildInfo, sizeof(EnumChildInfo));

		//���ñ���
		EnumChildInfo.pWndLayered = this;
		EnumChildInfo.pWndControl = m_pWndControl;
		EnumChildInfo.pRegionResult = &RegionResult;

		//ö�ٴ���
		ASSERT(m_pWndControl->GetSafeHwnd());
		EnumChildWindows(m_pWndControl->m_hWnd, EnumChildProc, (LPARAM)&EnumChildInfo);
	}

	//�����ų�
	if (!rcUnLayered.IsRectEmpty())
	{
		//��������
		CRgn RegionUnLayered;
		RegionUnLayered.CreateRoundRectRgn(rcUnLayered.left, rcUnLayered.top, rcUnLayered.right + 1, rcUnLayered.bottom + 1, PointRound.x, PointRound.y);

		//�ϲ�����
		RegionResult.CombineRgn(&RegionResult, &RegionUnLayered, RGN_DIFF);
	}

	//��������
	SetWindowRgn(RegionResult, TRUE);
}

//��������
VOID CLayeredWnd::InitLayeredArea(CImage& Image, BYTE cbAlpha,  const CRect& rcUnLayered, const CPoint& PointRound, BOOL bUnLayeredChild)
{
	//��������
	CImage ImageBuffer;
	ImageBuffer.Create(Image.GetWidth(), Image.GetHeight(), 32);

	//�滭����
	CImageDC ImageDC(ImageBuffer);
	CDC* pBufferDC = CDC::FromHandle(ImageDC);

	//�滭����
	ASSERT(pBufferDC);
	if (pBufferDC) Image.Draw(pBufferDC->GetSafeHdc(), 0, 0);

	//�����ֲ�
	InitLayeredArea(pBufferDC, cbAlpha, rcUnLayered, PointRound, bUnLayeredChild);

	return;
}

//�ر���Ϣ
VOID CLayeredWnd::OnClose()
{
	//Ͷ����Ϣ
	if (m_pWndControl)
	{
		m_pWndControl->PostMessage(WM_CLOSE);
	}
}

//������Ϣ
VOID CLayeredWnd::OnSetFocus(CWnd* pOldWnd)
{
	//���ý���
	if (m_pWndControl)
	{
		m_pWndControl->SetFocus();
	}
}

//�������
LRESULT CLayeredWnd::OnNcHitTest(CPoint point)
{
	return HTCAPTION;
}

//λ�øı�
void CLayeredWnd::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CWnd::OnWindowPosChanged(lpwndpos);
	m_pWndControl->MoveWindow(lpwndpos->x, lpwndpos->y, lpwndpos->cx, lpwndpos->cy);
}

//ö�ٺ���
BOOL CALLBACK CLayeredWnd::EnumChildProc(HWND hWndChild, LPARAM lParam)
{
	//��ȡλ��
	CRect rcWindow;
	::GetWindowRect(hWndChild, &rcWindow);

	//��������
	if ((rcWindow.Width() > 0) && (rcWindow.Height() > 0))
	{
		//��������
		ASSERT(lParam);
		tagEnumChildInfo* pEnumChildInfo = (tagEnumChildInfo *)lParam;

		//�����ж�
		HWND hWndParent = ::GetParent(hWndChild);
		if (hWndParent != pEnumChildInfo->pWndControl->GetSafeHwnd())
		{
			return TRUE;
		}

		//ת��λ��
		ASSERT(pEnumChildInfo->pWndControl);
		pEnumChildInfo->pWndControl->ScreenToClient(&rcWindow);

		//��������
		CRgn RgnChild;
		RgnChild.CreateRectRgn(rcWindow.left, rcWindow.top, rcWindow.right, rcWindow.bottom);

		//�ϲ�����
		ASSERT(pEnumChildInfo->pRegionResult);
		pEnumChildInfo->pRegionResult->CombineRgn(pEnumChildInfo->pRegionResult, &RgnChild, RGN_DIFF);
	}

	return TRUE;
}

