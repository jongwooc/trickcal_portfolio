#include "StdAfx.h"
#include "Ball.h"

#include "Include.h"
CBall::CBall(void){}

CBall::CBall( const OBJINFO& Info )
:CLogoDynamic(Info){}

CBall::~CBall(void)
{	Release();}

HRESULT CBall::Initialize(vector<TILE*>*	_pm_vecTile)
{	m_SortID = SORTID_SECOND;
	m_pObjKey = _BALL_;
	m_pStateKey = NULL;

	return S_OK;}

int CBall::Progress(const DWORD _input)
{	static float fTest;
	fTest += GET_SINGLE(CTimeMgr)->GetTime();
	m_fAngle += D3DXToRadian(90.f * GET_SINGLE(CTimeMgr)->GetTime());	

	D3DXMATRIX matTrans;
	D3DXMATRIX matRotZ;
	D3DXMatrixTranslation(&matTrans,m_Info.vPos.x,m_Info.vPos.y,m_Info.vPos.z);
	D3DXMatrixRotationZ(&matRotZ,m_fAngle);
	m_Info.matWorld = matRotZ * matTrans;

	if(fTest >3.f || _input)
	{	return SCENE_STAGE;	}
	
	return SCENE_NONPASSS;}

void CBall::Render()
{	const TEXINFO* pTexInfo = GET_SINGLE(CTextureMgr)->GetTexture(m_pObjKey);

	if(pTexInfo == NULL)
	{	return;	}

	m_Info.vCenter = D3DXVECTOR3(pTexInfo->ImgInfo.Width /2.f, pTexInfo->ImgInfo.Height/2.f	,0.f);

	GET_SINGLE(CDevice)->GetSprite()->SetTransform(&m_Info.matWorld);
	GET_SINGLE(CDevice)->GetSprite()->Draw(pTexInfo->pTexture,NULL
		,&m_Info.vCenter,NULL, D3DCOLOR_ARGB(255,255,255,255));}

void CBall::Release(){}

CObj* CBall::Clone()
{	return new CBall(*this);}


