#pragma once
#include "Defines.h"

class CObj
{
protected:
	OBJINFO m_Info;
	int	m_pObjKey;
	int  m_pStateKey;
protected:
	int	m_SortID;
	float	m_fFrame;
public:
	void Frame_Move(const float& fCnt, const float& fMax);
public:
	int GetSortID()
	{
		return m_SortID;
	}
public:
	virtual HRESULT Initialize(vector<TILE*>*	_pm_vecTile)PURE;
	virtual int Progress(const DWORD _input)PURE;
	virtual void Render()PURE;
	virtual void Release()PURE;
	virtual CObj* Clone()PURE;

public:
	CObj(void);
	CObj(const OBJINFO& Info);
	virtual ~CObj(void);
};

