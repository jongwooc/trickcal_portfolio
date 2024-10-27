#include "StdAfx.h"
#include "StageObjProto.h"

#include "Include.h"

CStageObjProto::CStageObjProto(void)
{
}


CStageObjProto::~CStageObjProto(void)
{
}

HRESULT CStageObjProto::InitProtoInstance()
{
	OBJINFO ObjInfo;

	ZeroMemory(&ObjInfo,sizeof(ObjInfo));

	D3DXMatrixIdentity(&ObjInfo.matWorld);
	ObjInfo.vPos  = D3DXVECTOR3(400.f,300.f,0.f);
	ObjInfo.vDir  = D3DXVECTOR3(0.f,0.f,0.f);
	ObjInfo.vLook = D3DXVECTOR3(1.f,0.f,0.f);

	//주소값으로 맵소팅
	m_MapProto.insert(make_pair(_BACKGROUND_,new CBackGround(ObjInfo)));


	m_MapProto.insert(make_pair(_PLAYER_,new CPlayer(ObjInfo)));

	return S_OK;
}
