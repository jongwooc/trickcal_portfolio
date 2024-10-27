#pragma once
#include "LogoStatic.h"

class CLogoBack:public CLogoStatic
{
public:
	virtual HRESULT Initialize(vector<TILE*>*	_pm_vecTile);
	virtual int Progress(const DWORD _input);
	virtual void Render();
	virtual void Release();
	virtual CObj* Clone();

public:
	CLogoBack(void);
	CLogoBack(const OBJINFO& Info);
	~CLogoBack(void);
};

