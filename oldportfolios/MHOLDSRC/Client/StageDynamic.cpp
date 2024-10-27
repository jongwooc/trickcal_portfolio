#include "StdAfx.h"
#include "StageDynamic.h"
#include "Include.h"


CStageDynamic::CStageDynamic(void)
:m_fAngle(0){}

CStageDynamic::CStageDynamic( const OBJINFO& Info )
:CObj(Info)
,m_fAngle(0){}

CStageDynamic::~CStageDynamic(void){}

void CStageDynamic::Chase_Move(const D3DXVECTOR3 & _DestVec)
{
	m_Info.vDir = _DestVec - m_Info.vPos;
	D3DXVec3Normalize(&m_Info.vDir,&m_Info.vDir);
	m_Info.vPos += m_Info.vDir * 100.f *GET_SINGLE(CTimeMgr)->GetTime();
}

D3DVECTOR CStageDynamic::Set_Direction(const DWORD & _Direction)
{
	switch(_Direction)
	{
	case KEY_LEFT:
		{	return D3DXVECTOR3(m_Info.vPos.x-100.f,m_Info.vPos.y,m_Info.vPos.z);	break;}
	case KEY_UP:
		{	return D3DXVECTOR3(m_Info.vPos.x,m_Info.vPos.y-100.f,m_Info.vPos.z);	break;}
	case KEY_RIGHT:
		{	return D3DXVECTOR3(m_Info.vPos.x+100.f,m_Info.vPos.y,m_Info.vPos.z);	break;}
	case KEY_DOWN:
		{	return D3DXVECTOR3(m_Info.vPos.x,m_Info.vPos.y+100,m_Info.vPos.z);	break;}
	}
}


