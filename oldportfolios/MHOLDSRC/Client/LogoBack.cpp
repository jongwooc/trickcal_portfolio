#include "StdAfx.h"
#include "LogoBack.h"

#include "Include.h"

CLogoBack::CLogoBack(void)
{
}

CLogoBack::CLogoBack( const OBJINFO& Info )
:CLogoStatic(Info)
{

}


CLogoBack::~CLogoBack(void)
{
	Release();
}

HRESULT CLogoBack::Initialize(vector<TILE*>*	_pm_vecTile)
{
	m_SortID = SORTID_FIRST;
	m_pObjKey = _LOGOBACK_;
	m_pStateKey = NULL;



	return S_OK;
}

int CLogoBack::Progress(const DWORD _input)
{
	D3DXMATRIX matTrans;

	D3DXMatrixTranslation(&matTrans,m_Info.vPos.x, m_Info.vPos.y, m_Info.vPos.z);

	m_Info.matWorld = matTrans;


	return SCENE_NONPASSS;
}

void CLogoBack::Render()
{
	const TEXINFO* pTexInfo = GET_SINGLE(CTextureMgr)->GetTexture(m_pObjKey);

	if(pTexInfo == NULL)
		return;

	m_Info.vCenter = D3DXVECTOR3(pTexInfo->ImgInfo.Width /2.f, pTexInfo->ImgInfo.Height/2.f
		,0.f);

	GET_SINGLE(CDevice)->GetSprite()->SetTransform(&m_Info.matWorld);
	GET_SINGLE(CDevice)->GetSprite()->Draw(pTexInfo->pTexture,NULL
		,&m_Info.vCenter,NULL, D3DCOLOR_ARGB(255,255,255,255));




}

void CLogoBack::Release()
{

}

CObj* CLogoBack::Clone()
{
	return new CLogoBack(*this);
}
