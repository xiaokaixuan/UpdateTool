#if !defined(_MESSAGE_H_)
#define _MESSAGE_H_

// ��������ͨ��
#define S_DISABLE 0x2 // SendMessage ����ֵ���������

// ��Ϣ����
enum
{
	UM_IFNEEDUPDATE   = WM_USER + 101,	// ѯ���Ƿ��������
	UM_NONEEDUPDATE   = WM_USER + 102,	// ������£���������
	UM_CHECKFILEERROR = WM_USER + 103,	// ����ļ�����
	UM_DISABLEUPDATE  = WM_USER + 104	// δ��������
};

#endif // _MESSAGE_H_