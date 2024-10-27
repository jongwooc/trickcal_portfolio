#pragma once
#include "Obj.h"
class CLogoDynamic:public CObj
{
protected:
	float		m_fAngle;
public:
	void Chase_Move(const OBJINFO& DestInfo);
	void AStar_Move(const OBJINFO& DestInfo);
public:
	virtual HRESULT Initialize(vector<TILE*>*	_pm_vecTile)PURE;
	virtual int Progress(const DWORD _input)PURE;
	virtual void Render()PURE;
	virtual void Release()PURE;
	virtual CObj* Clone()PURE;

public:
	CLogoDynamic(void);
	CLogoDynamic(const OBJINFO& Info);
	~CLogoDynamic(void);
};

