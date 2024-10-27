#include "StdAfx.h"
#include "BackGround.h"

CBackGround::CBackGround(void)
{
}

CBackGround::CBackGround( const OBJINFO& Info )
:CStageStatic(Info)
{

}


CBackGround::~CBackGround(void)
{
	Release();
}

HRESULT CBackGround::Initialize(vector<TILE*>*	_pm_vecTile)
{
	m_SortID = SORTID_FIRST;

	pm_vecTile = _pm_vecTile;
	m_pObjKey = _BACKGROUND_;
	m_pStateKey = _TILE_;
	D3DXMATRIX matTrans;





	for(unsigned int i =0; i<(*pm_vecTile).size(); ++i)
	{
	const TEXINFO* pTexInfo = GET_SINGLE(CTextureMgr)->GetTexture(m_pObjKey,
		m_pStateKey,(*pm_vecTile)[i]->byDrawId);

	if(pTexInfo == NULL)
		return S_FALSE;

/*

	D3DXMatrixTranslation(&matTrans,(*pm_vecTile)[i]->vPos.x,(*pm_vecTile)[i]->vPos.y,(*pm_vecTile)[i]->vPos.z);

	m_Info.matWorld = matTrans;
	m_Info.vCenter = D3DXVECTOR3(65.f,34.f,0.0f);

	GET_SINGLE(CDevice)->GetSprite()->SetTransform(&m_Info.matWorld);
	GET_SINGLE(CDevice)->GetSprite()->Draw(pTexInfo->pTexture
	,NULL,&D3DXVECTOR3(28,28,0),NULL, D3DCOLOR_ARGB(255,255,255,255));*/
	}

	return S_OK;
}

int CBackGround::Progress(const DWORD _input)
{




	return SCENE_NONPASSS;
}

void CBackGround::Render()
{
	D3DXMATRIX matTrans;




	for(unsigned int i =0; i<(*pm_vecTile).size(); ++i)
	{
		const TEXINFO* pTexInfo = GET_SINGLE(CTextureMgr)->GetTexture(m_pObjKey,
			m_pStateKey,(*pm_vecTile)[i]->byDrawId);

		if(pTexInfo == NULL)
			return;


		D3DXMatrixTranslation(&matTrans,(*pm_vecTile)[i]->vPos.x,(*pm_vecTile)[i]->vPos.y,(*pm_vecTile)[i]->vPos.z);

		m_Info.matWorld = matTrans;
		m_Info.vCenter = D3DXVECTOR3(28,28,0);

		GET_SINGLE(CDevice)->GetSprite()->SetTransform(&m_Info.matWorld);
		GET_SINGLE(CDevice)->GetSprite()->Draw(pTexInfo->pTexture,NULL,
			&m_Info.vCenter,NULL,D3DCOLOR_ARGB(255,255,255,255));

		



	}
}

void CBackGround::Release()
{
/*
	for(unsigned int i =0; i<(*pm_vecTile).size(); ++i)
	{SAFE_DELETE((*pm_vecTile)[i]);}*/
//	(pm_vecTile)->clear();
}

CObj* CBackGround::Clone()
{
	return new CBackGround(*this);
}

