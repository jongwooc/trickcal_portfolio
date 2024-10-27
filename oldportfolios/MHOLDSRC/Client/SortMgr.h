#pragma once
#include "Defines.h"
#include "Obj.h"

class CSortMgr
{
public:
	DECLARE_SINGLETON(CSortMgr);
private:
	list<CObj*>		m_ObjList[SORTID_END];

public:
	void AddSortObject(CObj* pObj);
	void RenderObject(void);
	void clearlist();

private:
	CSortMgr(void);
public:
	~CSortMgr(void);
};

