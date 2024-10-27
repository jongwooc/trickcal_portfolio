#pragma once
#include "Defines.h"

class CTexture;
class CTextureMgr
{
public:
	DECLARE_SINGLETON(CTextureMgr);
private:
	map<const int, CTexture*>	 m_MapTexture;
public:
	const TEXINFO* GetTexture(const int pObjKey
		,const int pStatKey = NULL, const int& iCnt =0);
public:
	HRESULT		InsertTexture(const TCHAR* pFileName
		,const int pObjKey, TEXTYPE TypeID
		,const int pStatKey = NULL, const int& iCnt =0);
	void Release();
public:
	CTextureMgr(void);
	~CTextureMgr(void);
};

