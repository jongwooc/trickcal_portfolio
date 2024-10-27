#include "StdAfx.h"
#include "TextMgr.h"


void CTextMgr::inputText(TCHAR * _szTmp)
{
//	GET_SINGLE(CTextMgr)->inputText(L"test1234");
	VecTempText.push_back(_szTmp);

}

void CTextMgr::Render()
{
	RECT	rcFont = {0,0,200,225};

	for(int i =0; i!= VecTempText.size(); i++)
	{	
		D3DXMatrixTranslation(&matTrans,700,25+(i*20),0);

		GET_SINGLE(CDevice)->GetSprite()->SetTransform(&matTrans);
		GET_SINGLE(CDevice)->GetFont()->DrawTextW(GET_SINGLE(CDevice)->GetSprite(),
		(VecTempText[i]),lstrlen(VecTempText[i]),&rcFont,DT_NOCLIP,D3DCOLOR_ARGB(255,75,0,150));	}


	for(int i =0; i != VecTempText.size(); i++)
	{		SAFE_DELETE(VecTempText[i]);	}
	VecTempText.clear();
	
}

CTextMgr::CTextMgr(void)
{
}


CTextMgr::~CTextMgr(void)
{
	for(int i =0; i != VecTempText.size(); i++)
	{		SAFE_DELETE(VecTempText[i]);	}

}
