#pragma once
//이미지 한장
#include "Texture.h"

class CSingleTexture:public CTexture
{
private:
	TEXINFO*		m_pTexInfo;
public:
	virtual const TEXINFO* GetTexture(const int & pStatKey = NULL, const int& iCnt =0);
public:
	virtual HRESULT InsertTexture(const TCHAR* pFileName
		,const int & pStatKey = NULL, const int& iCnt =0);
	virtual void Release();
public:
	CSingleTexture(void);
	~CSingleTexture(void);
};

