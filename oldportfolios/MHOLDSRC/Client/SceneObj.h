#pragma once
#include "Defines.h"

class CPrototype;
class SceneObj
{
protected:
	CPrototype* m_pPrototype;
public:
	virtual HRESULT Initialize()PURE;
	virtual void Progress(const DWORD _input)PURE;
	virtual void Render()PURE;
	virtual void Release()PURE;
public:
	SceneObj(void);
	virtual ~SceneObj(void);
};

