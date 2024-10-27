#pragma once
#include "Defines.h"
#include "Obj.h"
#include "Prototype.h"

class CObjMgr
{
public:
	DECLARE_SINGLETON(CObjMgr);
private:
	map<const int,list<CObj*>> m_MapObject;
	vector<OBJINFO*> vecOBJINFO;
//	vector<TILE*> * pvecTile;
public:
	HRESULT AddObject(CPrototype* pProto, const int pObjKey,vector<TILE*> * _pvecTile);

	int Progress(const DWORD _input);
	void Render();
	void Release();

private:
	CObjMgr(void);
public:
	~CObjMgr(void);
};

