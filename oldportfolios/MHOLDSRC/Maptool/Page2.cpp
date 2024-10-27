// Page2.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "Maptool.h"
#include "Page2.h"
#include "afxdialogex.h"
#include "MainFrm.h"
#include "MaptoolView.h"



// CPage2 ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CPage2, CPropertyPage)

CPage2::CPage2()
	: CPropertyPage(CPage2::IDD)
{

}

CPage2::~CPage2()
{
}

void CPage2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPage2, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON1, &CPage2::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CPage2::OnBnClickedButton2)
END_MESSAGE_MAP()


// CPage2 �޽��� ó�����Դϴ�.

//����
void CPage2::OnBnClickedButton1()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	CFileDialog Dlg(FALSE,L"dat",L"*.dat"
		,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,L"*.dat",this );


	if(Dlg.DoModal() == IDCANCEL)
		return;


	HANDLE hFile;
	DWORD  dwByte;

	hFile = CreateFile(Dlg.GetPathName(), 
		GENERIC_WRITE,0,NULL,CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL);

	CMainFrame*  pMainFrm = (CMainFrame*)AfxGetMainWnd();

	for(unsigned int i = 0; i< pMainFrm->m_pMainView->m_vecTile.size(); ++i)
	{
		WriteFile(hFile,pMainFrm->m_pMainView->m_vecTile[i],sizeof(TILE),&dwByte,NULL);
	}

	CloseHandle(hFile);
}


void CPage2::OnBnClickedButton2()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	CFileDialog Dlg(TRUE,L"dat",L"*.dat"
		,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,L"*.dat",this );


	if(Dlg.DoModal() == IDCANCEL)
		return;

	CMainFrame*  pMainFrm = (CMainFrame*)AfxGetMainWnd();

	HANDLE hFile;
	DWORD  dwByte;
	pMainFrm->m_pMainView->m_vecTile.clear();
	hFile = CreateFile(Dlg.GetPathName(), 
		GENERIC_READ,0,NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL);

	
	while(true)
	{
		TILE*	pTile = new TILE;

		ReadFile(hFile,pTile,sizeof(TILE),&dwByte,NULL);

		if(dwByte == 0)
		{
			SAFE_DELETE(pTile);
			break;
		}

		pMainFrm->m_pMainView->m_vecTile.push_back(pTile);
	}
	CloseHandle(hFile);

}
