#include "StdAfx.h"
#include "LogoDynamic.h"

#include "Include.h"

CLogoDynamic::CLogoDynamic(void)
:m_fAngle(0.f)
{
}

CLogoDynamic::CLogoDynamic( const OBJINFO& Info )
:CObj(Info)
,m_fAngle(0.f)
{

}


CLogoDynamic::~CLogoDynamic(void)
{
}

void CLogoDynamic::Chase_Move( const OBJINFO& DestInfo )
{
	m_Info.vDir = DestInfo.vPos - m_Info.vPos;

	D3DXVec3Normalize(&m_Info.vDir,&m_Info.vDir);

	m_Info.vPos += m_Info.vDir * 100.f *GET_SINGLE(CTimeMgr)->GetTime();



}

void CLogoDynamic::AStar_Move(const OBJINFO& DestInfo)
{

}
