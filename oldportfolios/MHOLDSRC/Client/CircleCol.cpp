#include "StdAfx.h"
#include "CircleCol.h"


void CircleCol::Initialize(void)
{

}

float CircleCol::Progress(void)
{
/*
	//반지름 + 반지름
	float fSum = pTemp->GetInfo().fCX/2 + pDest->GetInfo().fCX/2;

	//각 좌표의 거리
	float fWidth = pTemp->GetInfo().fX - pDest->GetInfo().fX;
	float fHeight = pTemp->GetInfo().fY - pDest->GetInfo().fY;

	float fDistance = sqrt(fWidth*fWidth + fHeight*fHeight);
	//반지름끼리 합이 더크면 충돌
	if(fSum > fDistance)
	{
		return (fSum - fDistance);
	}

	return false;*/
	return 0;
}

void CircleCol::Render(void)
{

}

void CircleCol::Release(void)
{

}

CircleCol::CircleCol(void)
{
}


CircleCol::~CircleCol(void)
{
}
