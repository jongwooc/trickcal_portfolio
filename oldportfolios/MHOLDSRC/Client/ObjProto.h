#pragma once
#include "Prototype.h"
#include "Obj.h"

class CObjProto:public CPrototype
{
protected:
	map<const int,CObj*> m_MapProto;
public:
	CObj* GetProto(const int pObjKey);
public:
	virtual HRESULT InitProtoInstance()PURE;
	virtual void Release();
public:
	CObjProto(void);
	~CObjProto(void);
};

