#pragma once
#include "Defines.h"

class CDevice
{
	DECLARE_SINGLETON(CDevice);
private:
	//장치의 성능을 조사한다. m_pDevice 생성해준다.
	LPDIRECT3D9			m_p3D;

	//장치를 대표하는 객체
	LPDIRECT3DDEVICE9   m_pDevice;

	LPD3DXSPRITE		m_pSprite;

public:
	LPD3DXSPRITE GetSprite()
	{
		return m_pSprite;
	}
	
	LPDIRECT3DDEVICE9 GetDevice()
	{
		return m_pDevice;
	}


public:
	HRESULT InitDevice(HWND hWnd);
public:
	void Render_Begin();
	void Render_End(HWND hWnd);
	void Release();

private:
	CDevice(void);
public:
	~CDevice(void);
};

