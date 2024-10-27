#pragma once
#include "LogoDynamic.h"

class CBall:public CLogoDynamic
{

public:
	virtual HRESULT Initialize(vector<TILE*>*	_pm_vecTile);
	virtual int Progress(const DWORD _input);
	virtual void Render();
	virtual void Release();
	virtual CObj* Clone();

public:
	CBall(void);
	CBall(const OBJINFO& Info);
	~CBall(void);
};

