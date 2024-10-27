#pragma once
#include "StageDynamic.h"
#include "Include.h"
class CPlayer:public CStageDynamic
{
private:
	vector<TILE*> *	pm_vecTile;
	vector<int>	pm_WallTile;
	D3DXVECTOR3		prePos;
	int CharTileId;

	LPCWSTR str;
	RECT rt;
	CString _int;

	vector <int> vecPretile;

	map <int, vector<int>>	mapRoute;
	map <int, vector<int>>	fastroute;


	void MoareRoad(int _TileNum, int _start);
	int AStar_Move(const int des);
	int GetCurrentTileid(const OBJINFO& CurrentInfo);
	vector <int> GetAdjacentTile(const int _tile);
	vector <int> GetAdjacentTile1(const int _tile);
	float MeasureDistance(const D3DXVECTOR3& CurrentInfo,const D3DXVECTOR3& DestInfo);
	int FollowRoad();

	
public:
	const OBJINFO SetMouseInfo();



public:
	virtual HRESULT Initialize(vector<TILE*>*	_pm_vecTile);
	virtual int Progress(const DWORD _input);
	virtual void Render();
	virtual void Release();
	virtual CObj* Clone();



public:
	CPlayer(void);
	CPlayer(const OBJINFO& Info);
	~CPlayer(void);
};

