#include "StdAfx.h"
#include "ObjProto.h"


CObjProto::CObjProto(void)
{
}


CObjProto::~CObjProto(void)
{
	Release();
}

void CObjProto::Release()
{
	for(map<const int,CObj*>::iterator iter = m_MapProto.begin(); 
		iter!= m_MapProto.end();++iter)
	{
		SAFE_DELETE(iter->second);
	}
	m_MapProto.clear();

}

CObj* CObjProto::GetProto( const int pObjKey )
{
	map<const int,CObj*>::iterator iter = m_MapProto.find(pObjKey);
	
	if(iter == m_MapProto.end())
	{
		ERR_MSG(g_hWnd,L"�����Ϸ��� ������ü�� �����ϴ�.");
		return NULL;
	}

	return iter->second;
}
