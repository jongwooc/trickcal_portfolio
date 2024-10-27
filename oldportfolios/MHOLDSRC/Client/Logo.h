#pragma once
#include "SceneObj.h"

class CLogo:public SceneObj
{
public:
	virtual HRESULT Initialize();
	virtual void Progress(const DWORD _input);
	virtual void Render();
	virtual void Release();
	int SceneID;
public:
	CLogo(void);
	~CLogo(void);
};

