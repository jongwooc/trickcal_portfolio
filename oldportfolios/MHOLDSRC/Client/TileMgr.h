#pragma once
#include "Include.h"

class CTileMgr
{
	DECLARE_SINGLETON(CTileMgr);
private:
	vector<TILE*>	m_vecTile;
public:
	void InitTileFromFile();
	vector<TILE*> * GetTilePointer(){return &m_vecTile;}
	void ClearTile();
public:
	CTileMgr(void);
	~CTileMgr(void);
};

