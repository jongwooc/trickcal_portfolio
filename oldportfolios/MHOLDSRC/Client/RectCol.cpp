#include "StdAfx.h"
#include "RectCol.h"


void RectCol::Initialize(void)
{

}

void RectCol::Progress(void)
{
	RECT rcCol;

/*
	if(IntersectRect(&rcCol,&GetRect(),&m_pBox->GetRect()))
	{
		SetRect(&rcCol,0,0,rcCol.right - rcCol.left,rcCol.bottom - rcCol.top);

		//������ �浹
		//Width < Height
		if(rcCol.right <rcCol.bottom)
		{
			//�ڽ��� ���ʿ��� �浹
			if(m_tInfo.fX< m_pBox->GetInfo().fX)
				m_tInfo.fX -= rcCol.right;
			else
				m_tInfo.fX += rcCol.right;


		}//�ؿ��� ������ �浹
		else
		{
			//�ؿ��� �浹
			if(m_tInfo.fY > m_pBox->GetInfo().fY)
				m_tInfo.fY += rcCol.bottom;
			else
				m_tInfo.fY -= rcCol.bottom;
		}

		RECT rc;

		for(list<CObj*>::iterator iter1 = pTemp->begin(); 
			iter1 != pTemp->end(); ++iter1)
		{
			for(list<CObj*>::iterator iter2 = pDest->begin();
				iter2 != pDest->end(); )
			{
				if(IntersectRect(&rc,&(*iter1)->GetRect(),&(*iter2)->GetRect()))
				{
					Safe_Delete(*iter1);
					Safe_Delete(*iter2);

					iter1 = pTemp->erase(iter1);
					iter2 = pDest->erase(iter2);
				}
				else
				{
					++iter2;
				}
				if(iter1 == pTemp->end())
					break;
			}
			if(iter1 == pTemp->end())
				break;
		}

	}
*/

}

void RectCol::Render(void)
{

}

void RectCol::Release(void)
{

}

RectCol::RectCol(void)
{
}


RectCol::~RectCol(void)
{
}
