#if !defined(AFX_CONFFILE_H__E7FB8C6C_3685_4147_B0A4_623BF30C33A7__INCLUDED_)
#define AFX_CONFFILE_H__E7FB8C6C_3685_4147_B0A4_623BF30C33A7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable: 4786)
#include <map>

#if !defined(_AFX)
#include <string>
typedef std::basic_string<TCHAR>	  _tstring;
#else
typedef CString _tstring;
#endif // !defined(_AFX)
typedef std::map<_tstring, _tstring> MapStringToString;

class CConfigFile  
{
public:
	CConfigFile(LPCTSTR lpFileName = NULL);
	virtual ~CConfigFile();

public:
	BOOL  GetSection(LPCTSTR lpAppName, MapStringToString& mapStrToStr);
	BOOL  WriteSection(LPCTSTR lpAppName, MapStringToString& mapStrToStr);
	int   GetKeyInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, int nDefault);
	BOOL  WriteKeyInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, int nNumber);
	float GetKeyFloat(LPCTSTR lpAppName, LPCTSTR lpKeyName, float fDefault);
	BOOL  WriteKeyFloat(LPCTSTR lpAppName, LPCTSTR lpKeyName, float fNumber);
	DWORD GetKeyString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, _tstring& strVal);
	DWORD GetKeyString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR szRetString, DWORD dwSize);
	BOOL  WriteKeyString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpString);

protected:
	_tstring m_strIni;
};

#endif // !defined(AFX_CONFFILE_H__E7FB8C6C_3685_4147_B0A4_623BF30C33A7__INCLUDED_)

