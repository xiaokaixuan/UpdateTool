#if !defined(_LAYEREDWND_H_32036F60_E63A_474b_B8E9_A6396A747634_INCLUDED)
#define _LAYEREDWND_H_32036F60_E63A_474b_B8E9_A6396A747634_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//分层窗口
class CLayeredWnd : public CWnd
{
	//变量定义
protected:
	CWnd *							m_pWndControl;						//控件窗口

	//函数定义
public:
	//构造函数
	CLayeredWnd();
	//析构函数
	virtual ~CLayeredWnd();

	//功能函数
public:
	//创建窗口
	VOID CreateLayered(CWnd* pWndControl, const CRect& rcWindow);
	//设置区域
	VOID InitLayeredArea(CDC* pDCImage, BYTE cbAlpha, const CRect& rcUnLayered, const CPoint& PointRound, BOOL bUnLayeredChild);
	//设置区域
	VOID InitLayeredArea(CImage& Image, BYTE cbAlpha, const CRect& rcUnLayered, const CPoint& PointRound, BOOL bUnLayeredChild);

	//消息映射
protected:
	//关闭消息
	VOID OnClose();
	//焦点消息
	VOID OnSetFocus(CWnd* pOldWnd);
	
	//点击测试
	LRESULT OnNcHitTest(CPoint point);
	//位置改变
	void OnWindowPosChanged(WINDOWPOS* lpwndpos);

	//静态函数
protected:
	//枚举函数
	static BOOL CALLBACK EnumChildProc(HWND hWndChild, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

#endif // !defined(_LAYEREDWND_H_32036F60_E63A_474b_B8E9_A6396A747634_INCLUDED)

