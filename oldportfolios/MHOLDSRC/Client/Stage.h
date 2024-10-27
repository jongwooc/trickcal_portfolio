#pragma once
#include "SceneObj.h"
class CStage:public SceneObj
{
private:
	vector<TILE*>*	pm_vecTile;
public:
	virtual HRESULT Initialize();
	virtual void Progress(const DWORD _input);
	virtual void Render();
	virtual void Release();
public:
	CStage(void);
	~CStage(void);
};

