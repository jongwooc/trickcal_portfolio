#pragma once
#include "Obj.h"

class CLogoStatic:public CObj
{

public:
	virtual HRESULT Initialize(vector<TILE*>*	_pm_vecTile)PURE;
	virtual int Progress(const DWORD _input)PURE;
	virtual void Render()PURE;
	virtual void Release()PURE;

public:
	CLogoStatic(void);
	CLogoStatic(const OBJINFO& Info);
	~CLogoStatic(void);
};

