#include "StdAfx.h"
#include "TileMgr.h"


void CTileMgr::InitTileFromFile()
{
	HANDLE hFile;
	DWORD  dwByte;

	hFile = CreateFile(L"../Data/Tile.dat",GENERIC_READ,
		0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

	while(true)
	{
		TILE*	pTile = new TILE;

		ReadFile(hFile,pTile,sizeof(TILE),&dwByte,NULL);

		if(dwByte == 0)
		{
			SAFE_DELETE(pTile);
			break;
		}

		m_vecTile.push_back(pTile);
	}
	CloseHandle(hFile);
}

void CTileMgr::ClearTile()
{
	m_vecTile.clear();

}

CTileMgr::CTileMgr(void)
{
}


CTileMgr::~CTileMgr(void)
{
	for(int i =0; i != m_vecTile.size(); i++)
	{		SAFE_DELETE(m_vecTile[i]);	}
}
