#include "StdAfx.h"
#include "Stage.h"

#include "Include.h"

CStage::CStage(void)
{
}


CStage::~CStage(void)
{
	Release();
}

HRESULT CStage::Initialize()
{
	GET_SINGLE(CTimeMgr)->InitTimeMgr();
	GET_SINGLE(CTileMgr)->InitTileFromFile();
	pm_vecTile = GET_SINGLE(CTileMgr)->GetTilePointer();
	

	//객체 생성
	m_pPrototype = new CStageObjProto;

	m_pPrototype->InitProtoInstance();


	if(FAILED(GET_SINGLE(CObjMgr)->AddObject(m_pPrototype,_BACKGROUND_,pm_vecTile)))
	{
		ERR_MSG(g_hWnd,L"객체생성실패");
		return E_FAIL;
	}



	if(FAILED(GET_SINGLE(CObjMgr)->AddObject(m_pPrototype,_PLAYER_,pm_vecTile)))
	{
		ERR_MSG(g_hWnd,L"객체생성실패");
		return E_FAIL;
	}



	if(FAILED(GET_SINGLE(CTextureMgr)->InsertTexture(L"../Resource/Texture/reimu/stand%d .bmp"
		,_PLAYER_,TEXTYPE_MUTI,_STAND_,10)))
	{
		ERR_MSG(g_hWnd,L"Player 실패");
		return E_FAIL;
	}
	if(FAILED(GET_SINGLE(CTextureMgr)->InsertTexture(L"../Resource/Texture/reimu/walkFront%d.bmp"
		,_PLAYER_1,TEXTYPE_MUTI,_WALK_,8)))
	{
		ERR_MSG(g_hWnd,L"Player 1실패");
		return E_FAIL;
	}

	if(FAILED(GET_SINGLE(CTextureMgr)->InsertTexture(L"../Resource/Texture/Tile/Tile%d.png"
		,_BACKGROUND_,TEXTYPE_MUTI,_TILE_,22)))
	{
		ERR_MSG(g_hWnd,L"Stage Tile 실패");
		return E_FAIL;
	}

/*
	if(FAILED(m_pPrototype->InitProtoInstance()))
	{
		ERR_MSG(g_hWnd,L"원형객체 생성실패");
		return E_FAIL;
	}*/

	return S_OK;
}

void CStage::Progress(const DWORD _input)
{
	GET_SINGLE(CTimeMgr)->SetTime();

	int SceneID = GET_SINGLE(CObjMgr)->Progress(_input);


	if(SceneID >= 0)
	{
		GET_SINGLE(CSceneMgr)->InitScene(SceneID);

	int	SceneID = GET_SINGLE(CObjMgr)->Progress(_input);
		return;
	}

}

void CStage::Render()
{
	GET_SINGLE(CObjMgr)->Render();
}

void CStage::Release()
{
	DESTROY_SINGLE(CObjMgr);
	DESTROY_SINGLE(CTextureMgr);
	SAFE_DELETE(m_pPrototype);
	DESTROY_SINGLE(CTimeMgr);
}
