#include "StdAfx.h"
#include "ObjMgr.h"

#include "Include.h"

CObjMgr::CObjMgr(void)
{
}


CObjMgr::~CObjMgr(void)
{
	Release();
}

HRESULT CObjMgr::AddObject( CPrototype* pProto, const int pObjKey,vector<TILE*> * _pvecTile )
{
	map<const int,list<CObj*>>::iterator iter = m_MapObject.find(pObjKey);

	CObj*  pProtoInst = ((CObjProto*)pProto)->GetProto(pObjKey);

	if(pProtoInst == NULL)
		return E_FAIL;

	CObj* pObject = pProtoInst->Clone();
	pObject->Initialize(_pvecTile);

	if(iter == m_MapObject.end())
	{
		list<CObj*>	Objlist;
		Objlist.push_back(pObject);

		m_MapObject.insert(make_pair(pObjKey,Objlist));
	}
	else
	{
		iter->second.push_back(pObject);
	}

	
	return S_OK;
}

int CObjMgr::Progress(const DWORD _input)
{
	for(map<const int,list<CObj*>>::iterator iter = m_MapObject.begin(); 
		iter != m_MapObject.end(); ++iter)
	{
		for(list<CObj*>::iterator iter1 = iter->second.begin();
			iter1 != iter->second.end(); ++iter1)
		{
			int SceneID = (*iter1)->Progress(_input);

			
			if(SceneID >=0)
			{GET_SINGLE(CSortMgr)->clearlist();
			return SceneID;}


			GET_SINGLE(CSortMgr)->AddSortObject(*iter1);
		}
	}
	vecOBJINFO.clear();
	return SCENE_NONPASSS;
}

void CObjMgr::Render()
{

	GET_SINGLE(CSortMgr)->RenderObject();


	/*for(map<const int,list<CObj*>>::iterator iter = m_MapObject.begin(); 
		iter != m_MapObject.end(); ++iter)
	{
		for(list<CObj*>::iterator iter1 = iter->second.begin();
			iter1 != iter->second.end(); ++iter1)
		{
			(*iter1)->Render();
		}
	}*/
}

void CObjMgr::Release()
{
	for(map<const int,list<CObj*>>::iterator iter = m_MapObject.begin(); 
		iter != m_MapObject.end(); ++iter)
	{
		for(list<CObj*>::iterator iter1 = iter->second.begin();
			iter1 != iter->second.end(); ++iter1)
		{
			SAFE_DELETE(*iter1);
		}
		iter->second.clear();
	}
	m_MapObject.clear();
}

