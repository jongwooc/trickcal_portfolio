#pragma once
//이미지 여러장
#include "Texture.h"

class CMultiTexture:public CTexture
{
private:
	//Player : walk : 01234
	//Player : attack : 0123

	map<const int,vector<TEXINFO*>> m_MapTexture;
public:
	virtual const TEXINFO* GetTexture(const int & pStatKey = NULL, const int& iCnt = 0);
public:
	virtual HRESULT InsertTexture(const TCHAR* pFileName
		,const int & pStatKey = NULL, const int& iCnt =0);
	virtual void Release();
public:
	CMultiTexture(void);
	~CMultiTexture(void);
};

