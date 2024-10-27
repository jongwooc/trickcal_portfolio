#pragma once
#include "Obj.h"

class CStageStatic:public CObj
{
public:
	virtual HRESULT Initialize(vector<TILE*>*	_pm_vecTile)PURE;
	virtual int Progress(const DWORD _input)PURE;
	virtual void Render()PURE;
	virtual void Release()PURE;
	virtual CObj* Clone()PURE;

public:
	CStageStatic(void);
	CStageStatic(const OBJINFO& Info);
	~CStageStatic(void);
};

