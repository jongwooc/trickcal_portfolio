// MySheet.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "Maptool.h"
#include "MySheet.h"


// MySheet

IMPLEMENT_DYNAMIC(MySheet, CPropertySheet)

MySheet::MySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{

}

MySheet::MySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{

}

MySheet::MySheet()
{
	AddPage(&m_Page1);
	AddPage(&m_Page2);
}

MySheet::~MySheet()
{
}


BEGIN_MESSAGE_MAP(MySheet, CPropertySheet)
END_MESSAGE_MAP()


// MySheet �޽��� ó�����Դϴ�.
