#if !defined(_LAYEREDWND_H_32036F60_E63A_474b_B8E9_A6396A747634_INCLUDED)
#define _LAYEREDWND_H_32036F60_E63A_474b_B8E9_A6396A747634_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//�ֲ㴰��
class CLayeredWnd : public CWnd
{
	//��������
protected:
	CWnd *							m_pWndControl;						//�ؼ�����

	//��������
public:
	//���캯��
	CLayeredWnd();
	//��������
	virtual ~CLayeredWnd();

	//���ܺ���
public:
	//��������
	VOID CreateLayered(CWnd* pWndControl, const CRect& rcWindow);
	//��������
	VOID InitLayeredArea(CDC* pDCImage, BYTE cbAlpha, const CRect& rcUnLayered, const CPoint& PointRound, BOOL bUnLayeredChild);
	//��������
	VOID InitLayeredArea(CImage& Image, BYTE cbAlpha, const CRect& rcUnLayered, const CPoint& PointRound, BOOL bUnLayeredChild);

	//��Ϣӳ��
protected:
	//�ر���Ϣ
	VOID OnClose();
	//������Ϣ
	VOID OnSetFocus(CWnd* pOldWnd);
	
	//�������
	LRESULT OnNcHitTest(CPoint point);
	//λ�øı�
	void OnWindowPosChanged(WINDOWPOS* lpwndpos);

	//��̬����
protected:
	//ö�ٺ���
	static BOOL CALLBACK EnumChildProc(HWND hWndChild, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

#endif // !defined(_LAYEREDWND_H_32036F60_E63A_474b_B8E9_A6396A747634_INCLUDED)

