#pragma once
#include "Obj.h"

class CStageDynamic:public CObj
{
protected:
	float		m_fAngle;
	vector<TILE*>	pm_prevecTile;
public:
	void Chase_Move(const D3DXVECTOR3 & _DestVec);
//	void AStar_Move(const OBJINFO& DestInfo);
	D3DVECTOR Set_Direction(const DWORD & _Direction);
public:
	virtual HRESULT Initialize(vector<TILE*>*	_pm_vecTile)PURE;
	virtual int Progress(const DWORD _input)PURE;
	virtual void Render()PURE;
	virtual void Release()PURE;
	virtual CObj* Clone()PURE;

public:
	CStageDynamic(void);
	CStageDynamic(const OBJINFO& Info);
	~CStageDynamic(void);
};

