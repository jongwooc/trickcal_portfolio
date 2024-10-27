#pragma once
#include "Defines.h"

class SceneObj;
class CSceneMgr
{
public:
	DECLARE_SINGLETON(CSceneMgr);
private:
	SceneObj*  m_pSceneObj;
public:
	HRESULT InitScene(int Scene);
	void Progress(const DWORD _input);
	void Render();
	void Release();

private:
	CSceneMgr(void);
public:
	~CSceneMgr(void);
};

