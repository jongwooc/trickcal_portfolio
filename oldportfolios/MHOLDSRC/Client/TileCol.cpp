#include "StdAfx.h"
#include "TileCol.h"


int CTileCol::ObjCol(float _TileCenterX,float _TileCenterY, float _TileSizeHalfX, float _TileSizeHalfY, float _ObjCenterX,float _ObjCenterY, float _ObjSizeHalfX, float _ObjSizeHalfY)
{
	if (
		((abs(_ObjCenterX-_TileCenterX)) + abs(_ObjCenterY-_TileSizeHalfY) ) <TILESIZEY
		){return 1;}
	else 
	{	return 0;}
}

CTileCol::CTileCol(void)
{
}


CTileCol::~CTileCol(void)
{
}
