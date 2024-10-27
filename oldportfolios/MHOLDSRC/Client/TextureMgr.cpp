#include "StdAfx.h"
#include "TextureMgr.h"

#include "Texture.h"
#include "Include.h"


CTextureMgr::CTextureMgr(void)
{
}


CTextureMgr::~CTextureMgr(void)
{
	Release();
}

HRESULT CTextureMgr::InsertTexture( const TCHAR* pFileName ,const int pObjKey, 
	TEXTYPE TypeID,const int & pStatKey , const int& iCnt )
{
	map<const int,CTexture*>::iterator iter = m_MapTexture.find(pObjKey);

	if(iter == m_MapTexture.end())
	{
		CTexture*	pTexture = NULL;

		switch(TypeID)
		{
		case TEXTYPE_SINGLE:
			pTexture = new CSingleTexture;
			break;
		case TEXTYPE_MUTI:
			pTexture = new CMultiTexture;
			break;
		}
		if(FAILED(pTexture->InsertTexture(pFileName,pStatKey,iCnt)))
			return E_FAIL;

		m_MapTexture.insert(make_pair(pObjKey,pTexture));
	}

	return S_OK;
}

const TEXINFO* CTextureMgr::GetTexture( const int pObjKey,const int & pStatKey , const int& iCnt )
{
	map<const int, CTexture*>::iterator iter = m_MapTexture.find(pObjKey);

	if(iter == m_MapTexture.end())
		return NULL;

	return iter->second->GetTexture(pStatKey,iCnt);
}

void CTextureMgr::Release()
{
	for(map<const int ,CTexture*>::iterator iter = m_MapTexture.begin();
		iter != m_MapTexture.end(); ++iter)
	{
		delete iter->second;
		iter->second = NULL;
	}

	m_MapTexture.clear();
}
