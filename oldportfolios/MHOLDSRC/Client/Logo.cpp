#include "StdAfx.h"
#include "Logo.h"

#include "Include.h"

CLogo::CLogo(void)
{
}


CLogo::~CLogo(void)
{
	Release();
}

HRESULT CLogo::Initialize()
{

	GET_SINGLE(CTimeMgr)->InitTimeMgr();



	//객체를 생성한다.
	m_pPrototype = new CLogoObjProto;

	if(FAILED(m_pPrototype->InitProtoInstance()))
	{
		ERR_MSG(g_hWnd,L"원형객체생성실패");
		return E_FAIL;
	}

	if(FAILED(GET_SINGLE(CObjMgr)->AddObject(m_pPrototype,_LOGOBACK_,NULL)))
	{
		ERR_MSG(g_hWnd,L"객체생성실패");
		return E_FAIL;
	}

	if(FAILED(GET_SINGLE(CObjMgr)->AddObject(m_pPrototype,_BALL_,NULL)))
	{
		ERR_MSG(g_hWnd,L"객체생성실패");
		return E_FAIL;
	}

	if(FAILED(GET_SINGLE(CTextureMgr)->InsertTexture(L"../Resource/Texture/Logo/Ball.png"
		,_BALL_,TEXTYPE_SINGLE)))
	{
		ERR_MSG(g_hWnd,L"Ball 실패");
		return E_FAIL;
	}

	if(FAILED(GET_SINGLE(CTextureMgr)->InsertTexture(L"../Resource/Texture/Logo/LogoBack.png"
		,_LOGOBACK_,TEXTYPE_SINGLE)))
	{
		ERR_MSG(g_hWnd,L"LogoBack 실패");
		return E_FAIL;
	}



	
	return S_OK;
}

void CLogo::Progress(const DWORD _input)
{
	GET_SINGLE(CTimeMgr)->SetTime();
	SceneID =  GET_SINGLE(CObjMgr)->Progress(_input);

	if(SceneID >= 0)
	{
		GET_SINGLE(CSceneMgr)->InitScene(SceneID);
//		SceneID =  GET_SINGLE(CObjMgr)->Progress();
		return;
	}

}

void CLogo::Render()
{
	GET_SINGLE(CObjMgr)->Render();
}

void CLogo::Release()
{
	DESTROY_SINGLE(CObjMgr);
	DESTROY_SINGLE(CTextureMgr);
	SAFE_DELETE(m_pPrototype);
	DESTROY_SINGLE(CTimeMgr);
}
