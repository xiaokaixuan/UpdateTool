#include "StdAfx.h"
#include <tchar.h>
#include <Shlwapi.h>
#include "ConfigFile.h"

#pragma comment(lib, "Shlwapi.lib")
#pragma warning(disable: 4996)

#define _MAX_SITE 4096
#define _SEC_SIZE (4*1024*1024)

inline LPCTSTR c_str(const _tstring &str)
{
#if !defined(_AFX)
	return str.c_str();
#else
	return LPCTSTR(str);
#endif
}

inline DWORD length(const _tstring &str)
{
#if !defined(_AFX)
	return DWORD(str.length());
#else
	return DWORD(str.GetLength());
#endif
}

CConfigFile::CConfigFile(LPCTSTR lpFileName/* = NULL*/)
{
	TCHAR szFileName[_MAX_SITE] = {0};
	if (lpFileName)
	{
		::GetFullPathName(lpFileName, _MAX_SITE, szFileName, NULL);
	}
	else
	{
		::GetModuleFileName(NULL, szFileName, _MAX_SITE);
		::PathRenameExtension(szFileName, _T(".cfg"));
	}
	m_strIni = szFileName;
}

CConfigFile::~CConfigFile()
{

}

BOOL CConfigFile::GetSection(LPCTSTR lpAppName, MapStringToString& mapStrToStr)
{
	size_t nMapSize = mapStrToStr.size();
	DWORD dwLen = _SEC_SIZE;
	LPTSTR pString = new TCHAR[dwLen];
	ZeroMemory(pString, dwLen*sizeof(TCHAR));
	DWORD dwRet = ::GetPrivateProfileSection(lpAppName, pString, dwLen, c_str(m_strIni));
	for (LPTSTR pStr(pString); pStr < pString + dwRet;)
	{
		DWORD dwLength = lstrlen(pStr);

		LPTSTR pChar = _tcschr(pStr, _T('='));
		if (pChar)
		{
			int nLen = int(pChar - pStr);
			TCHAR szKey[_MAX_SITE] = {0};
			TCHAR szVal[_MAX_SITE] = {0};
			lstrcpyn(szKey, pStr, (nLen + 1) > _MAX_SITE ? _MAX_SITE : (nLen + 1));
			lstrcpyn(szVal, pChar + 1, _MAX_SITE);
			StrTrim(szKey, _T(" \t\n"));
			StrTrim(szVal, _T(" \t\n"));
			mapStrToStr[szKey] = szVal;
		}
		pStr += dwLength + 1;
	}
	delete[] pString;
	return mapStrToStr.size() > nMapSize;
}

int CConfigFile::GetKeyInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, int nDefault)
{
	return (int)::GetPrivateProfileInt(lpAppName, lpKeyName, nDefault, c_str(m_strIni));
}

float CConfigFile::GetKeyFloat(LPCTSTR lpAppName, LPCTSTR lpKeyName, float fDefault)
{
	float fRet(fDefault);
	TCHAR szStr[_MAX_SITE] = {0};
	::GetPrivateProfileString(lpAppName, lpKeyName, _T(""), szStr, _MAX_SITE, c_str(m_strIni));
	if (_stscanf(szStr, _T("%f"), &fRet) < 1)
	{
		fRet = fDefault;
	}
	return fRet;
}

DWORD CConfigFile::GetKeyString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, _tstring& strVal)
{
	DWORD dwRet(FALSE);
	TCHAR szString[_MAX_SITE] = {0};
	dwRet = GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, szString, _MAX_SITE, c_str(m_strIni));
	if (dwRet > 0)
	{
		strVal = szString;
	}
	return dwRet;
}

DWORD CConfigFile::GetKeyString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR szRetString, DWORD dwSize)
{
	return ::GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, szRetString, dwSize, c_str(m_strIni));
}

BOOL CConfigFile::WriteSection(LPCTSTR lpAppName, MapStringToString& mapStrToStr)
{
	BOOL bRet(FALSE);
	DWORD dwSize(0);
	for (MapStringToString::iterator i = mapStrToStr.begin(); i != mapStrToStr.end(); ++i)
	{
		dwSize += length(i->first) + length(i->second) + 1 + 1; // '=' and '\0'
	}
	if (dwSize > 0)
	{
		dwSize += 1; // "\0\0" end.

		LPTSTR szData = new TCHAR[dwSize];
		ZeroMemory(szData, dwSize*sizeof(TCHAR));

		LPTSTR szStr(szData);
		for (MapStringToString::iterator iter = mapStrToStr.begin(); iter != mapStrToStr.end(); ++iter)
		{
			DWORD dwLen = length(iter->first) + length(iter->second) + 1;
			_tstring str(iter->first);
			str += _T("=");
			str += iter->second;
			lstrcpyn(szStr, c_str(str), dwLen + 1);
			szStr += dwLen + 1;
		}
		bRet = ::WritePrivateProfileSection(lpAppName, szData, c_str(m_strIni));
		delete[] szData;
	}
	return bRet;
}

BOOL CConfigFile::WriteKeyInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, int nNumber)
{
	BOOL bRet(FALSE);
	TCHAR szNumber[32] = {0};
	_sntprintf(szNumber, 32, _T("%d"), nNumber);
	bRet = ::WritePrivateProfileString(lpAppName, lpKeyName, szNumber, c_str(m_strIni));
	return bRet;
}

BOOL CConfigFile::WriteKeyFloat(LPCTSTR lpAppName, LPCTSTR lpKeyName, float fNumber)
{
	BOOL bRet(FALSE);
	TCHAR szNumber[256] = {0};
	_sntprintf(szNumber, 256, _T("%f"), fNumber);
	bRet = ::WritePrivateProfileString(lpAppName, lpKeyName, szNumber, c_str(m_strIni));
	return bRet;
}

BOOL CConfigFile::WriteKeyString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpString)
{
	BOOL bRet(FALSE);
	bRet = ::WritePrivateProfileString(lpAppName, lpKeyName, lpString, c_str(m_strIni));
	return bRet;
}
