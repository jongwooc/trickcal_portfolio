#include "StdAfx.h"
#include "SortMgr.h"

#include "Include.h"

CSortMgr::CSortMgr(void)
{
}


CSortMgr::~CSortMgr(void)
{
}

void CSortMgr::AddSortObject( CObj* pObj )
{




	int	ID = pObj->GetSortID();

	m_ObjList[ID].push_back(pObj);
	
}

void CSortMgr::RenderObject( void )
{
	for(int i =0; i< SORTID_END; ++i)
	{
		for(list<CObj*>::iterator iter = m_ObjList[i].begin(); 
			iter != m_ObjList[i].end(); ++iter)
		{
			(*iter)->Render();
		}
//	
			m_ObjList[i].clear();	
	}

}

void CSortMgr::clearlist()
{
	for(int i =0; i< SORTID_END; ++i)
	{

		m_ObjList[i].clear();	
	}
}

