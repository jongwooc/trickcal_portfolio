#pragma once
#include "Include.h"
class CTextMgr
{
public:
	DECLARE_SINGLETON(CTextMgr);
private:
	vector <TCHAR*>	VecTempText;

	D3DXMATRIX  matTrans;

public:
	void inputText(TCHAR * _szTmp);
	void Render();


public:
	CTextMgr(void);
	~CTextMgr(void);
};

