#if !defined(_TIMER_H_BA815DBF_18E5_45e7_85CA_945D3DBF1BAB)
#define _TIMER_H_BA815DBF_18E5_45e7_85CA_945D3DBF1BAB

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CTimer
{
public:
	__forceinline CTimer()
	{
		QueryPerformanceCounter(&m_lnStart);
		QueryPerformanceFrequency(&m_lnFrequency);
	}
	__forceinline double Current() //当前计时值
	{
		LARGE_INTEGER lnCurrent;
		QueryPerformanceCounter(&lnCurrent);
		return (lnCurrent.QuadPart - m_lnStart.QuadPart) / double(m_lnFrequency.QuadPart);
	}
	__forceinline double Reset()  //重置计时器
	{
		LARGE_INTEGER lnCurrent;
		QueryPerformanceCounter(&lnCurrent);
		double dInterval = (lnCurrent.QuadPart - m_lnStart.QuadPart) / double(m_lnFrequency.QuadPart);
		m_lnStart = lnCurrent;
		return dInterval;
	}
private:
	LARGE_INTEGER m_lnStart;
	LARGE_INTEGER m_lnFrequency;
};

#endif // !defined(_TIMER_H_BA815DBF_18E5_45e7_85CA_945D3DBF1BAB)

