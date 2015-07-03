#if !defined(_UTILITY_H_)
#define _UTILITY_H_

// 像素转换
inline void CrossImage(CImage &img)
{
	for(int i=0; i<img.GetWidth(); i++)
	{
		for(int j=0; j<img.GetHeight(); j++)
		{
			UCHAR *cr = (UCHAR*) img.GetPixelAddress(i,j);
			cr[0] = cr[0]*cr[3] / 255;
			cr[1] = cr[1]*cr[3] / 255;
			cr[2] = cr[2]*cr[3] / 255;
		}
	}
}

// 递归创建文件夹
inline BOOL CreateRankDirectory(LPCTSTR lpPath)
{
	BOOL bRet(FALSE);
	CString strPath(lpPath);
	strPath.Replace(_T('/'), _T('\\'));
	if (!PathFileExists(strPath))
	{
		CString strTemp;
		int pos(-1);
		while (-1 != (pos=strPath.Find('\\')))
		{
			strTemp += strPath.Left(pos + 1);
			strPath = strPath.Right(strPath.GetLength() - pos - 1);
			CreateDirectory(strTemp, NULL);
		}
		strTemp += strPath;
		CreateDirectory(strTemp, NULL);
		bRet = TRUE;
	}
	return bRet;
}

// 移动文件
inline void SHMoveFile(LPCTSTR pszSrcFile, LPCTSTR pszDestFile)
{
	if (PathFileExists(pszDestFile))
	{
		if (DeleteFile(pszDestFile))
		{
			MoveFile(pszSrcFile, pszDestFile);
		}
	}
	else
	{
		CString strFolder(pszDestFile);
		PathRemoveFileSpec(strFolder.GetBuffer(512));
		strFolder.ReleaseBuffer();
		if (PathFileExists(strFolder))
		{
			if (PathIsDirectory(strFolder))
			{
				MoveFile(pszSrcFile, pszDestFile);
			}
		}
		else
		{
			CreateRankDirectory(strFolder);
			MoveFile(pszSrcFile, pszDestFile);
		}
	}
}


#endif // !defined(_UTILITY_H_)

