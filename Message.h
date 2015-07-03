#if !defined(_MESSAGE_H_)
#define _MESSAGE_H_

// 与主进程通信
#define S_DISABLE 0x2 // SendMessage 返回值，允许更新

// 消息定义
enum
{
	UM_IFNEEDUPDATE   = WM_USER + 101,	// 询问是否允许更新
	UM_NONEEDUPDATE   = WM_USER + 102,	// 无需更新，已是最新
	UM_CHECKFILEERROR = WM_USER + 103,	// 检查文件错误
	UM_DISABLEUPDATE  = WM_USER + 104	// 未开启更新
};

#endif // _MESSAGE_H_