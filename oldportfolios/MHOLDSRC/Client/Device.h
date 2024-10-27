#pragma once
#include "Defines.h"

class CDevice
{
public:
	DECLARE_SINGLETON(CDevice);
private:
	//��ġ�� ������ �����Ѵ�. m_pDevice �������ش�.
	LPDIRECT3D9			m_p3D;

	//��ġ�� ��ǥ�ϴ� ��ü
	LPDIRECT3DDEVICE9   m_pDevice;

	LPD3DXSPRITE		m_pSprite;

	LPD3DXFONT			m_pFont;

	LPD3DXLINE			m_pLine;

public:
	LPD3DXFONT	GetFont()		{	return m_pFont;	}
	LPD3DXSPRITE GetSprite()	{	return m_pSprite;	}
	LPDIRECT3DDEVICE9 GetDevice(){	return m_pDevice;	}


public:
	HRESULT InitDevice(WINMODE Mode);
public:
	void Render_Begin();
	void Render_End();
	void Release();

private:
	CDevice(void);
public:
	~CDevice(void);
};

