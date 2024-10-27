#pragma once

#include "ObjProto.h"

class CStageObjProto:public CObjProto
{
public:
	virtual HRESULT InitProtoInstance();
public:
	CStageObjProto(void);
	~CStageObjProto(void);
};

