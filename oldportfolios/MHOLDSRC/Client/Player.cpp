#include "StdAfx.h"
#include "Player.h"
#include "Include.h"


CPlayer::CPlayer(void)
{
}

CPlayer::CPlayer( const OBJINFO& Info )
:CStageDynamic(Info)
{

}


CPlayer::~CPlayer(void)
{
	Release();
}

HRESULT CPlayer::Initialize(vector<TILE*>*	_pm_vecTile)
{
	m_SortID = SORTID_SECOND;
	m_pObjKey = _PLAYER_;
	m_pStateKey = _STAND_;
	pm_vecTile = _pm_vecTile;

	for(unsigned int i =0; i<(*pm_vecTile).size(); ++i)
	{	switch ((*pm_vecTile)[i]->byOption)
		{case 1000:
			{	pm_WallTile.push_back(i);}	}	}

	return S_OK;
}

int CPlayer::Progress(const DWORD _input)
{
	Frame_Move(10,8);

	TCHAR * szTmpx = new TCHAR[32];
	TCHAR * szMousex = new TCHAR[32];
	TCHAR * chartile = new TCHAR[32];
	TCHAR * mousetile = new TCHAR[32];
	TCHAR * AdTile = new TCHAR[32];
	int mousetileid = 0;
	CharTileId = GetCurrentTileid(m_Info);
	mousetileid = GetCurrentTileid(SetMouseInfo());

	if ((*pm_vecTile)[mousetileid]->byOption>500)
	{
		vector <int> temp = GetAdjacentTile1(mousetileid);
		for (int i = 0; i != temp.size(); i++)
		{	
			for(int j = 0; j != temp.size()-2; j++ )
			{	
				if	(((*pm_vecTile)[temp[j]]->byOption
		>	(*pm_vecTile)[temp[j+1]]->byOption)
		||	((MeasureDistance((*pm_vecTile)[temp[j]]->vPos,m_Info.vPos)
		>	MeasureDistance((*pm_vecTile)[temp[j+1]]->vPos,m_Info.vPos))
		&&	((*pm_vecTile)[temp[j]]->byOption
		=	(*pm_vecTile)[temp[j+1]]->byOption)	)	)		
				{	int temp1 = temp[j+1];
				temp[j+1] = temp[j];
				temp[j] = temp1;	}	}
		}
		mousetileid = temp[0];	

	}

	_stprintf (szTmpx,L"케릭터좌표: %0.f/%0.f",m_Info.vPos.x,m_Info.vPos.y);
	GET_SINGLE(CTextMgr)->inputText(szTmpx);
	_stprintf (chartile,L"캐릭타일: %d",GetCurrentTileid(m_Info));
	GET_SINGLE(CTextMgr)->inputText(chartile);

	_stprintf (szMousex,L"마우스좌표: %0.f/%0.f",SetMouseInfo().vPos.x,SetMouseInfo().vPos.y);
	GET_SINGLE(CTextMgr)->inputText(szMousex);
	_stprintf (mousetile,L"마우스타일: %d",mousetileid);
	GET_SINGLE(CTextMgr)->inputText(mousetile);
//	_stprintf (AdTile,L"%d/%d/%d/%d/%d/%d",(GetAdjacentTile(mousetileid)[0]),(GetAdjacentTile(mousetileid)[1]),(GetAdjacentTile(mousetileid)[2]),(GetAdjacentTile(mousetileid)[3]),(GetAdjacentTile(mousetileid)[4]),(GetAdjacentTile(mousetileid)[5]));
//	GET_SINGLE(CTextMgr)->inputText(AdTile);

	switch(_input)
	{
	case KEY_LBUTTON:
	{	fastroute.clear();
		if ((*pm_vecTile)[mousetileid]->byOption < 100)
		{	Chase_Move((*pm_vecTile)[mousetileid]->vPos);
			Frame_Move(8,8);
			m_pObjKey = _PLAYER_1;
			m_pStateKey = _WALK_;	}
			break;	}
	case KEY_RBUTTON:
	{	fastroute.clear();
		int test1 = AStar_Move(mousetileid);	
		if (test1 != 0)
		{	MoareRoad(test1,mousetileid);	}
	break;	}
	case KEY_LEFT:
	{	fastroute.clear();	
		int WallSwitch = 1;
		Frame_Move(8,8);
		m_pObjKey = _PLAYER_1;
		m_pStateKey = _WALK_;
		for (int i = 0; i<pm_WallTile.size(); i++)
		{		int test99 = pm_WallTile[i];
		if (MeasureDistance(D3DXVECTOR3(m_Info.vPos.x-18,m_Info.vPos.y,m_Info.vPos.z),(*pm_vecTile)[pm_WallTile[i]]->vPos) <29)
		{	WallSwitch = 0;	}	}
		if (WallSwitch)
		{	Chase_Move(Set_Direction(KEY_LEFT));	}
		break;	}
	case KEY_UP:
	{	fastroute.clear();	
		int WallSwitch = 1;
		Frame_Move(8,8);
		m_pObjKey = _PLAYER_1;
		m_pStateKey = _WALK_;
		for (int i = 0; i<pm_WallTile.size(); i++)
		{		int test99 = pm_WallTile[i];
		if (MeasureDistance(D3DXVECTOR3(m_Info.vPos.x,m_Info.vPos.y-18,m_Info.vPos.z),(*pm_vecTile)[pm_WallTile[i]]->vPos) <29)
		{	WallSwitch = 0;	}	}
		if (WallSwitch)
		{	Chase_Move(Set_Direction(KEY_UP));	}
		break;	}
	case KEY_RIGHT:
	{	fastroute.clear();
		int WallSwitch = 1;
		Frame_Move(8,8);
		m_pObjKey = _PLAYER_1;
		m_pStateKey = _WALK_;
		for (int i = 0; i<pm_WallTile.size(); i++)
		{		int test99 = pm_WallTile[i];
		if (MeasureDistance(D3DXVECTOR3(m_Info.vPos.x+18,m_Info.vPos.y,m_Info.vPos.z),(*pm_vecTile)[pm_WallTile[i]]->vPos) <29)
		{	WallSwitch = 0;	}	}
		if (WallSwitch)
		{	Chase_Move(Set_Direction(KEY_RIGHT));	}
		break;	}
	case KEY_DOWN:
	{	fastroute.clear();
		int WallSwitch = 1;
		Frame_Move(8,8);
		m_pObjKey = _PLAYER_1;
		m_pStateKey = _WALK_;
		for (int i = 0; i<pm_WallTile.size(); i++)
		{		int test99 = pm_WallTile[i];
		if (MeasureDistance(D3DXVECTOR3(m_Info.vPos.x,m_Info.vPos.y+18,m_Info.vPos.z),(*pm_vecTile)[pm_WallTile[i]]->vPos) <29)
		{	WallSwitch = 0;	}	}
		if (WallSwitch)
		{	Chase_Move(Set_Direction(KEY_DOWN));	}
		break;	}
	case (KEY_LEFT + KEY_UP):
	{	fastroute.clear();	
		int WallSwitch = 1;
		Frame_Move(8,8);
		m_pObjKey = _PLAYER_1;
		m_pStateKey = _WALK_;
		for (int i = 0; i<pm_WallTile.size(); i++)
		{		int test99 = pm_WallTile[i];
		if (MeasureDistance(D3DXVECTOR3(m_Info.vPos.x-18,m_Info.vPos.y-18,m_Info.vPos.z),(*pm_vecTile)[pm_WallTile[i]]->vPos) <29)
		{	WallSwitch = 0;	}	}
		if (WallSwitch)
		{	Chase_Move(Set_Direction(KEY_LEFT));
			Chase_Move(Set_Direction(KEY_UP));	}
		break;	}

	case (KEY_UP + KEY_RIGHT):
	{	fastroute.clear();	
		int WallSwitch = 1;
		Frame_Move(8,8);
		m_pObjKey = _PLAYER_1;
		m_pStateKey = _WALK_;
		for (int i = 0; i<pm_WallTile.size(); i++)
		{		int test99 = pm_WallTile[i];
		if (MeasureDistance(D3DXVECTOR3(m_Info.vPos.x+18,m_Info.vPos.y-18,m_Info.vPos.z),(*pm_vecTile)[pm_WallTile[i]]->vPos) <29)
		{	WallSwitch = 0;	}	}
		if (WallSwitch)
		{	Chase_Move(Set_Direction(KEY_UP));
			Chase_Move(Set_Direction(KEY_RIGHT));	}
		break;	}


	case (KEY_RIGHT + KEY_DOWN):
	{	fastroute.clear();	
		int WallSwitch = 1;
		Frame_Move(8,8);
		m_pObjKey = _PLAYER_1;
		m_pStateKey = _WALK_;
		for (int i = 0; i<pm_WallTile.size(); i++)
		{		int test99 = pm_WallTile[i];
		if (MeasureDistance(D3DXVECTOR3(m_Info.vPos.x+18,m_Info.vPos.y+18,m_Info.vPos.z),(*pm_vecTile)[pm_WallTile[i]]->vPos) <29)
		{	WallSwitch = 0;	}	}
		if (WallSwitch)
		{	Chase_Move(Set_Direction(KEY_RIGHT));
			Chase_Move(Set_Direction(KEY_DOWN));	}
		break;	}


	case (KEY_DOWN + KEY_LEFT):
	{	fastroute.clear();		
		int WallSwitch = 1;
		Frame_Move(8,8);
		m_pObjKey = _PLAYER_1;
		m_pStateKey = _WALK_;
		for (int i = 0; i<pm_WallTile.size(); i++)
		{		int test99 = pm_WallTile[i];
		if (MeasureDistance(D3DXVECTOR3(m_Info.vPos.x-18,m_Info.vPos.y+18,m_Info.vPos.z),(*pm_vecTile)[pm_WallTile[i]]->vPos) <29)
		{	WallSwitch = 0;	}	}
		if (WallSwitch)
		{	Chase_Move(Set_Direction(KEY_DOWN));
			Chase_Move(Set_Direction(KEY_LEFT));	}
		break;	}
	
	}

	FollowRoad();


	D3DXMATRIX	matTrans;

	D3DXMatrixTranslation(&matTrans,m_Info.vPos.x,m_Info.vPos.y,m_Info.vPos.z);
	
	m_Info.matWorld = matTrans;





	return SCENE_NONPASSS;
}

void CPlayer::Render()
{

	const TEXINFO* pTexInfo = GET_SINGLE(CTextureMgr)->GetTexture(m_pObjKey,m_pStateKey,int(m_fFrame));

	if(pTexInfo == NULL)
		return;

	m_Info.vCenter = D3DXVECTOR3(pTexInfo->ImgInfo.Width *(0.35), pTexInfo->ImgInfo.Height*(0.8)
		,0.f);

	GET_SINGLE(CDevice)->GetSprite()->SetTransform(&m_Info.matWorld);
	GET_SINGLE(CDevice)->GetSprite()->Draw(pTexInfo->pTexture,NULL
		,&m_Info.vCenter,NULL, D3DCOLOR_ARGB(255,255,255,255));




}

void CPlayer::Release()
{

}

CObj* CPlayer::Clone()
{
	return new CPlayer(*this);
}

const OBJINFO CPlayer::SetMouseInfo()
{
	POINT		ptMouse;

	GetCursorPos(&ptMouse);

	ScreenToClient(g_hWnd,&ptMouse);

	OBJINFO MouseInfo;

	ZeroMemory(&MouseInfo,sizeof(OBJINFO));

	MouseInfo.vPos = D3DXVECTOR3(float(ptMouse.x),float(ptMouse.y),0.f);
	if (MouseInfo.vPos.x<0)
	{	MouseInfo.vPos.x =0;	}
	if (MouseInfo.vPos.y<0)
	{	MouseInfo.vPos.y =0;	}
	return MouseInfo;
}



void CPlayer::MoareRoad(int _TileNum, int _start)
{

	vector <int> ShortRoad;
//	vector <int> FastRoad;
	int Road1 = 0;
	ShortRoad.push_back(_start);
	vector <int> temp = GetAdjacentTile1(_start);


	for (int i = 0; i != temp.size(); i++)
	{	
		for(int j = 0; j != temp.size()-2; j++ )
		{	
			if	(((*pm_vecTile)[temp[j]]->byOption
		>	(*pm_vecTile)[temp[j+1]]->byOption)
		||	((MeasureDistance((*pm_vecTile)[temp[j]]->vPos,m_Info.vPos)
		>	MeasureDistance((*pm_vecTile)[temp[j+1]]->vPos,m_Info.vPos))
		&&	((*pm_vecTile)[temp[j]]->byOption
		=	(*pm_vecTile)[temp[j+1]]->byOption)	)	)		
			{	int temp1 = temp[j+1];
				temp[j+1] = temp[j];
				temp[j] = temp1;	}	}
	}

	int RoadSpeed = 0;
	vector <int> temp1;
	temp1.reserve(6);

	for (int i = mapRoute.size()-1; i != -1; i--)
	{	
		for (int j = 0; j < mapRoute.at(i).size(); j++)
		{	
			for (int n = 0; n < temp.size(); n++)
			{
				if (temp[n] == mapRoute.at(i)[j])
				{	ShortRoad.push_back(temp[n]);	
					RoadSpeed +=(*pm_vecTile)[temp[n]]->byOption;
					temp1 = GetAdjacentTile1(temp[n]);
					for (int l = 0; l != temp1.size(); l++)
					{	
						for(int m = 0; m != temp1.size()-2; m++ )
						{	
							if	(((*pm_vecTile)[temp1[m]]->byOption
								>	(*pm_vecTile)[temp1[m+1]]->byOption)
								||	((MeasureDistance((*pm_vecTile)[temp1[m]]->vPos,m_Info.vPos)
								>	MeasureDistance((*pm_vecTile)[temp1[m+1]]->vPos,m_Info.vPos))
								&&	((*pm_vecTile)[temp1[m]]->byOption
								=	(*pm_vecTile)[temp1[m+1]]->byOption)	)	)		
							{	int temp00 = temp1[m+1];
								temp1[m+1] = temp1[m];
								temp1[m] = temp00;	}	}	}	
					temp.clear();
					temp = temp1;
					temp1.clear();
					n = 5000;
					j = 5000;
					break;	}	}	}	}	
	fastroute.insert(make_pair(RoadSpeed,ShortRoad));
	RoadSpeed = 0;
	mapRoute.clear();

}

int CPlayer::AStar_Move(const int des)
{
	vecPretile.clear();
	vector <int> vecStarttile;
	vector <int> tempvec;

	vecStarttile.push_back(GetCurrentTileid(m_Info));
	mapRoute.insert(make_pair(0,vecStarttile));
	if (des == GetCurrentTileid(m_Info))
	{return 0;}
		
	tempvec = 	GetAdjacentTile(vecStarttile[0]);

	for (int i = 0; i < tempvec.size();i++ )
	{	if (des == tempvec[i])
	{	tempvec.clear();
		tempvec.push_back(des);
		break;	}	}

	mapRoute.insert(make_pair(1,tempvec));
	for (int i = 0; i!= mapRoute.at(1).size();i++ )
	{	if (des == mapRoute.at(1)[i])
	{	return 1;	}
	}
	vecPretile.push_back(vecStarttile[0]);

	for (int i = 1; i!= tempvec.size(); i++)
	{	vecPretile.push_back(tempvec[i]);	}

	vecStarttile.clear();
	tempvec.clear();

	for (int i = 1; i!= min(50,mapRoute.size()) ; i++)
	{	for (int j= 0; j != mapRoute.at(i).size(); j++)
	{	int _temp = GetAdjacentTile(mapRoute.at(i)[j]).size();
		for (int k =0; k < _temp; k++)
		{	if (GetAdjacentTile(mapRoute.at(i)[j])[k] != -1)
			{	if (des == GetAdjacentTile(mapRoute.at(i)[j])[k])
				{	map <int, vector<int>>::iterator it;

					it=mapRoute.find((i+1));
					mapRoute.erase ( it, mapRoute.end() );
					vector <int> vecEndtile;
					vecEndtile.push_back(des);
					mapRoute.insert(make_pair(i+1,vecEndtile));
					return i;	}
				tempvec.push_back(GetAdjacentTile(mapRoute.at(i)[j])[k]);		
				vecPretile.push_back(GetAdjacentTile(mapRoute.at(i)[j])[k]);
				k--;}	}	}

	mapRoute.insert(make_pair(i+1,tempvec));
	tempvec.clear();	}
	
	return 0;
}

int CPlayer::GetCurrentTileid(const OBJINFO& CurrentInfo)
{

	for(unsigned int i =0; i<(*pm_vecTile).size()-1; ++i)
	{	if(	sqrtf(	pow(((*pm_vecTile)[i]->vPos.x-CurrentInfo.vPos.x),2) + 	pow(((*pm_vecTile)[i]->vPos.y+2-CurrentInfo.vPos.y),2)	) <30)
		{return i;}
	}
}

float CPlayer::MeasureDistance(const D3DXVECTOR3& CurrentInfo,const D3DXVECTOR3& DestInfo)
{	return (sqrtf(pow((CurrentInfo.x-DestInfo.x),2)+pow((CurrentInfo.y-DestInfo.y),2)));	}

int CPlayer::FollowRoad()
{
	CharTileId = GetCurrentTileid(m_Info);
	float speed =0;


	if (fastroute.empty())
	{	return 2;	}

	map <int, vector<int>>::iterator it1;
	it1=fastroute.begin();
	
	if (it1->second.empty())
	{	return 3;	}
	vector<int>::iterator it2;
	it2 = it1->second.end()-1;
	
	m_Info.vDir = (*pm_vecTile)[(*it2)]->vPos - m_Info.vPos;
	D3DXVec3Normalize(&m_Info.vDir,&m_Info.vDir);
	int test1 = GetCurrentTileid(m_Info);
	m_Info.vPos += m_Info.vDir * 80.f * GET_SINGLE(CTimeMgr)->GetTime() ;
	
	if (it2 == it1->second.begin())
	{	if (abs(m_Info.vPos.x - (*pm_vecTile)[(*it2)]->vPos.x) < 3 && abs(m_Info.vPos.y == (*pm_vecTile)[(*it2)]->vPos.y)<3)
		{	fastroute.clear();
			return 0;	}	}
	if (abs(m_Info.vPos.x - (*pm_vecTile)[(*it2)]->vPos.x) < 3 && abs(m_Info.vPos.y == (*pm_vecTile)[(*it2)]->vPos.y)<3)
	{	it2--;	it1->second.pop_back();	}
	return 1;
	
}

vector <int> CPlayer::GetAdjacentTile(const int _tile)
{
	vector<int> adjacent;
	adjacent.reserve(6);


	switch (int (_tile*0.03125)%2)
	{
	case 0:
		{	adjacent.push_back(_tile-32);
			adjacent.push_back(_tile-31);
			adjacent.push_back(_tile-1);
			adjacent.push_back(_tile+1);
			adjacent.push_back(_tile+32);
			adjacent.push_back(_tile+33);
			
			for(int i = 0; i != adjacent.size(); i++ )
			{	if	((adjacent[i] <0)
				||	((*pm_vecTile)[adjacent[i]]->byOption	>	500))
				{	adjacent.erase (adjacent.begin()+i);	i--;	}	}	
		break;}
	case 1:
		{	adjacent.push_back(_tile-33);
			adjacent.push_back(_tile-32);
			adjacent.push_back(_tile-1);
			adjacent.push_back(_tile+1);
			adjacent.push_back(_tile+31);
			adjacent.push_back(_tile+32);

			for(int i = 0; i != adjacent.size(); i++ )
			{	if	((adjacent[i] <0)
				||	((*pm_vecTile)[adjacent[i]]->byOption	>	500))
				{	adjacent.erase (adjacent.begin()+i);	i--	;	}	}	
		break;}	
	}


	for (int j = 0; j != adjacent.size(); j++)
	{	for (int i = 0; i != vecPretile.size(); i++)
		{	if	(adjacent[j] == vecPretile[i])
			{	adjacent[j] = -1;}	}	}


/*

	for(int i = 0; i != adjacent.size(); i++ )
	{	for(int j = 0; j != adjacent.size(); j++ )
	{	if	(((*pm_vecTile)[adjacent[j]]->byOption
		>	(*pm_vecTile)[adjacent[j+j]]->byOption)
		||	((MeasureDistance((*pm_vecTile)[adjacent[j]]->vPos,(*pm_vecTile)[des]->vPos)
		>	MeasureDistance((*pm_vecTile)[adjacent[j+1]]->vPos,(*pm_vecTile)[des]->vPos))
		&&	((*pm_vecTile)[adjacent[j]]->byOption
		=	(*pm_vecTile)[adjacent[j+j]]->byOption)	)	)
			{	int temp = adjacent[j+1];
				adjacent[j+1] = adjacent[j];
				adjacent[j] = temp;	}	}	}






	for(int i = 0; i != min(4,adjacent.size()); i++ )
	{	tricell.push_back(adjacent[i]);	}*/

	return adjacent;


}

vector <int> CPlayer::GetAdjacentTile1(const int _tile)
{
	vector<int> adjacent;
	adjacent.reserve(6);
	
	switch (int (_tile*0.03125)%2)
	{
	case 0:
		{	adjacent.push_back(_tile-32);
			adjacent.push_back(_tile-31);
			adjacent.push_back(_tile-1);
			adjacent.push_back(_tile+1);
			adjacent.push_back(_tile+32);
			adjacent.push_back(_tile+33);
			
			for(int i = 0; i != adjacent.size(); i++ )
			{	if	((adjacent[i] <0)
				||	((*pm_vecTile)[adjacent[i]]->byOption	>	500))
				{	adjacent.erase (adjacent.begin()+i);	i--;	}	
			}	
		break;}
	case 1:
		{	adjacent.push_back(_tile-33);
			adjacent.push_back(_tile-32);
			adjacent.push_back(_tile-1);
			adjacent.push_back(_tile+1);
			adjacent.push_back(_tile+31);
			adjacent.push_back(_tile+32);

			for(int i = 0; i != adjacent.size(); i++ )
			{	if	((adjacent[i] <0)
				||	((*pm_vecTile)[adjacent[i]]->byOption	>	500))
				{	adjacent.erase (adjacent.begin()+i);	i--	;	}	
			}	
		break;}	
	}
	
	return adjacent;
}

