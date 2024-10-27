#include "StdAfx.h"
#include "MultiTexture.h"
#include "Device.h"


CMultiTexture::CMultiTexture(void)
{
}


CMultiTexture::~CMultiTexture(void)
{
	Release();
}


HRESULT CMultiTexture::InsertTexture( const TCHAR* pFileName 
	,const int & pStatKey /*= NULL*/, const int& iCnt /*=0*/ )
{
	TCHAR szPath[128] = L"";

	vector<TEXINFO*> vecTexture;

	for(int i =0; i< iCnt; ++i)
	{
		wsprintf(szPath,pFileName, i);

		TEXINFO* pTexInfo = new TEXINFO;
		ZeroMemory(pTexInfo, sizeof(TEXINFO));


		if(FAILED(D3DXGetImageInfoFromFile(szPath,&pTexInfo->ImgInfo)))
		{
			return E_FAIL;
		}

		if(FAILED(D3DXCreateTextureFromFileEx(GET_SINGLE(CDevice)->GetDevice()
			,szPath,pTexInfo->ImgInfo.Width
			,pTexInfo->ImgInfo.Height,pTexInfo->ImgInfo.MipLevels
			,0,pTexInfo->ImgInfo.Format
			,D3DPOOL_MANAGED,D3DX_DEFAULT,D3DX_DEFAULT
			,D3DCOLOR_ARGB(255,0,123,139)
			,&pTexInfo->ImgInfo
			,NULL,&pTexInfo->pTexture)))
			return E_FAIL;

		vecTexture.push_back(pTexInfo);
	}

	m_MapTexture.insert(make_pair(pStatKey,vecTexture));

	return S_OK;
}



const TEXINFO* CMultiTexture::GetTexture( const int & pStatKey /*= NULL*/
	, const int& iCnt /*=0*/ )
{
	map<const int,vector<TEXINFO*>>::iterator iter = m_MapTexture.find(pStatKey);

	if(iter == m_MapTexture.end())
		return NULL;

	return iter->second[iCnt];
}

void CMultiTexture::Release()
{
	for(map<const int, vector<TEXINFO*>>::iterator iter = m_MapTexture.begin();
		iter != m_MapTexture.end(); ++iter)
	{
		for(unsigned int i =0; i< iter->second.size(); ++i)
		{
			iter->second[i]->pTexture->Release();
			delete iter->second[i];
			iter->second[i] = NULL;
		}
		iter->second.clear();
	}
	m_MapTexture.clear();


}
