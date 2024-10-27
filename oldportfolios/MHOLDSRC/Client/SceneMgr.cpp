#include "StdAfx.h"
#include "SceneMgr.h"

#include "Include.h"


CSceneMgr::CSceneMgr(void)
:m_pSceneObj(NULL)
{
}


CSceneMgr::~CSceneMgr(void)
{
	Release();
}

HRESULT CSceneMgr::InitScene( int Scene )
{
	SAFE_DELETE(m_pSceneObj);


	switch(Scene)
	{
	case SCENE_LOGO:
		m_pSceneObj = new CLogo;
		break;
	case SCENE_STAGE:
		m_pSceneObj = new CStage;
		break;
	}

	if(FAILED(m_pSceneObj->Initialize()))
	{
		SAFE_DELETE(m_pSceneObj);
		return E_FAIL;
	}
	
	return S_OK;
}

void CSceneMgr::Progress(const DWORD _input)
{
	m_pSceneObj->Progress(_input);
}

void CSceneMgr::Render()
{
	m_pSceneObj->Render();
}

void CSceneMgr::Release()
{
	SAFE_DELETE(m_pSceneObj);
}

