#pragma once
#include "Defines.h"

class CTexture
{
public:
	virtual const TEXINFO* GetTexture(const int pStatKey = NULL, const int& iCnt =0)PURE;
public:
	virtual HRESULT InsertTexture(const TCHAR* pFileName
		,const int pStatKey = NULL, const int& iCnt =0)PURE;
	virtual void Release()PURE;
public:
	CTexture(void);
	virtual ~CTexture(void);
};

