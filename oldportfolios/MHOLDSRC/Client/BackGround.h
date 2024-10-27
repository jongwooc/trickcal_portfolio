#pragma once
#include "StageStatic.h"
#include "Include.h"


class CBackGround:public CStageStatic
{
private:
	vector<TILE*>*		pm_vecTile;
	vector<OBJINFO*>	m_vecObj;

	LPCWSTR str;

	RECT rt;

	CString _int;



public:
	virtual HRESULT Initialize(vector<TILE*>*	_pm_vecTile);
	virtual int Progress(const DWORD _input);
	virtual void Render();
	virtual void Release();
	virtual CObj* Clone();

public:
	CBackGround(void);
	CBackGround(const OBJINFO& Info);
	~CBackGround(void);
};

