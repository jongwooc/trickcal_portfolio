
// MaptoolView.cpp : CMaptoolView Ŭ������ ����
//

#include "stdafx.h"
// SHARED_HANDLERS�� �̸� ����, ����� �׸� �� �˻� ���� ó���⸦ �����ϴ� ATL ������Ʈ���� ������ �� ������
// �ش� ������Ʈ�� ���� �ڵ带 �����ϵ��� �� �ݴϴ�.
#ifndef SHARED_HANDLERS
#include "Maptool.h"
#endif

#include "MaptoolDoc.h"
#include "MaptoolView.h"
#include "MainFrm.h"
#include "MiniView.h"
#include "Include.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMaptoolView

IMPLEMENT_DYNCREATE(CMaptoolView, CScrollView)

BEGIN_MESSAGE_MAP(CMaptoolView, CScrollView)
	// ǥ�� �μ� ����Դϴ�.
	ON_COMMAND(ID_FILE_PRINT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMaptoolView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

// CMaptoolView ����/�Ҹ�

CMaptoolView::CMaptoolView()
{
	// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.

}

CMaptoolView::~CMaptoolView()
{
}

BOOL CMaptoolView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs�� �����Ͽ� ���⿡��
	//  Window Ŭ���� �Ǵ� ��Ÿ���� �����մϴ�.

	return CScrollView::PreCreateWindow(cs);
}

// CMaptoolView �׸���

void CMaptoolView::OnDraw(CDC* pDC)
{
	CMaptoolDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: ���⿡ ���� �����Ϳ� ���� �׸��� �ڵ带 �߰��մϴ�.
	

	GET_SINGLE(CDevice)->Render_Begin();
	//for(unsigned int i = 0; i<m_vecTile.size(); ++i)
	D3DXMATRIX matWorld;
	D3DXMATRIX matTrans;

	for(int i =0; i<17; ++i)
	{
		for(int j=0; j<17; ++j)
		{
			int iIndex = (i+ GetScrollPos(1)/(TILESIZEY)) * TILECNTX +(j+GetScrollPos(0)/TILESIZEX);

			if(iIndex >= (TILECNTX) * (TILECNTY) || iIndex <0)
				continue;

			const TEXINFO* pTexInfo = GET_SINGLE(CTextureMgr)->GetTexture(_BACKGROUND_,
				_TILE_,m_vecTile[iIndex]->byDrawId);
			if(pTexInfo == NULL)
				return;




			D3DXMatrixTranslation(&matTrans,m_vecTile[iIndex]->vPos.x -GetScrollPos(0),
				m_vecTile[iIndex]->vPos.y-GetScrollPos(1), m_vecTile[iIndex]->vPos.z);

			matWorld = matTrans;

			GET_SINGLE(CDevice)->GetSprite()->SetTransform(&matWorld);
			GET_SINGLE(CDevice)->GetSprite()->Draw(pTexInfo->pTexture
				,NULL,&D3DXVECTOR3(28,28,0),NULL, D3DCOLOR_ARGB(255,255,255,255));


		}
	}
	GET_SINGLE(CDevice)->Render_End(m_hWnd);

	(*pDC).SelectStockObject(NULL_BRUSH);
	CFont font;

	VERIFY(font.CreatePointFont(30, _T("Malgun Gothic"), pDC));
	CFont* def_font = (*pDC).SelectObject(&font);
	(*pDC).SelectObject(def_font);

	if(GetKeyState(VK_CONTROL)&0x8001)
	{



		for(unsigned int i = 0; i<m_vecTile.size(); ++i)
		{

			RECT rt;
			CString _int;

			m_vecTile[i]->rowid = int(i*0.03125) %2;

			rt.left =		(m_vecTile[i]->vPos.x+4) - 30;
			rt.top = 		(m_vecTile[i]->vPos.y+6) - 10;
			rt.right =		(m_vecTile[i]->vPos.x+4) + 30;
			rt.bottom =		(m_vecTile[i]->vPos.y+6) + 15;


			_int.Format(_T("%d/%d/%d"),m_vecTile[i]->rowid,i,m_vecTile[i]->byOption );

			(*pDC).Ellipse((m_vecTile[i]->vPos.x+4) - (31), (m_vecTile[i]->vPos.y+6) - (28),(m_vecTile[i]->vPos.x+4) + (32),(m_vecTile[i]->vPos.y+6) + (28));
			//	(*pDC).Ellipse((m_vecTile[iIndex]->vPos.x) - 28, (m_vecTile[iIndex]->vPos.y) - 28,(m_vecTile[iIndex]->vPos.x) + 28,(m_vecTile[iIndex]->vPos.y) + 28);
			(*pDC).DrawText(_int,&rt,DT_CENTER|DT_VCENTER|DT_WORDBREAK);


			//724, 668, 1024, 768);

		}
	}
	font.DeleteObject();

//	(*pDC).Ellipse((m_vecTile[230]->vPos.x+4) - (31*10), (m_vecTile[230]->vPos.y+6) - (28*10),(m_vecTile[230]->vPos.x+4) + (32*10),(m_vecTile[230]->vPos.y+6) + (28*10));

	CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();

	pMainFrm->m_pMiniView->Invalidate(FALSE);

}


// CMaptoolView �μ�


void CMaptoolView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CMaptoolView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// �⺻���� �غ�
	return DoPreparePrinting(pInfo);
}

void CMaptoolView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: �μ��ϱ� ���� �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
}

void CMaptoolView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: �μ� �� ���� �۾��� �߰��մϴ�.
}

void CMaptoolView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CMaptoolView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CMaptoolView ����

#ifdef _DEBUG
void CMaptoolView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CMaptoolView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CMaptoolDoc* CMaptoolView::GetDocument() const // ����׵��� ���� ������ �ζ������� �����˴ϴ�.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMaptoolDoc)));
	return (CMaptoolDoc*)m_pDocument;
}
#endif //_DEBUG


// CMaptoolView �޽��� ó����


void CMaptoolView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	SetScrollSizes(MM_TEXT,CSize(TILESIZEX * (TILECNTX+2),
		(TILESIZEY-12)*(TILECNTY)));

	m_iSelDrawID = 0;


	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if(FAILED(GET_SINGLE(CDevice)->InitDevice(m_hWnd)))
	{
		return;
	}

/*
	if(FAILED(GET_SINGLE(CTextureMgr)->InsertTexture(L"../Resource/Texture/Tile/Tile21.png"
		,L"Test",TEXTYPE_SINGLE)))
	{
		AfxMessageBox(L"�̹����ε����");
		return;
	}*/

	if(FAILED(GET_SINGLE(CTextureMgr)->InsertTexture(L"../Resource/Texture/Tile/Tile%d.png"
		,_BACKGROUND_,TEXTYPE_MUTI,_TILE_,22)))
	{
		AfxMessageBox(L"��Ƽ�̹����ε� ����");
		return;
	}
	
	InitTile();

	CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();

	RECT rcWindow;

	pMainFrm->GetWindowRect(&rcWindow);
	
	SetRect(&rcWindow,0,0,rcWindow.right - rcWindow.left,rcWindow.bottom - rcWindow.top);

	RECT rcMainView;

	GetClientRect(&rcMainView);

	float	fRowFrm = rcWindow.bottom - rcMainView.bottom;
	float	fColFrm = rcWindow.right - rcMainView.right;

	pMainFrm->SetWindowPos(NULL,0,0,800+fColFrm, 600+ fRowFrm, SWP_NOZORDER);
	

}

void CMaptoolView::InitTile()
{
	m_vecTile.reserve(TILECNTX * TILECNTY);

	for(int y =0; y< TILECNTY; ++y)
	{
		for(int x=0; x <TILECNTX; ++x)
		{
			int iIndex = y * TILECNTX + x;

			TILE* pTile = new TILE;
			//���ͺ� ���� ¥����
			pTile->vPos = D3DXVECTOR3(x*TILESIZEX - ((y%2)* 28), y*(TILESIZEY-16.f), 0.f);
			pTile->vSize = D3DXVECTOR3(TILESIZEX,TILESIZEY,0.f);
			pTile->byDrawId = 0;
			pTile->byOption = 3;
			pTile->rowid = -1;
			m_vecTile.push_back(pTile);
		}
	}
}

void CMaptoolView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
		
	int iIndex =m_vecTile.size()-2;

	D3DXVECTOR2	vMousePoint = D3DXVECTOR2(point.x+GetScrollPos(0),point.y+GetScrollPos(1));

	for(int i =0; i<m_vecTile.size(); ++i)
	{
		if(CollisionMouseToTile(vMousePoint,m_vecTile[i]))
		{

			iIndex = i;
			break;
		}
	}
	m_vecTile[iIndex]->byDrawId = m_iSelDrawID;
	switch(m_vecTile[iIndex]->byDrawId)
	{
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
		{	m_vecTile[iIndex]->byOption = PASSABLE;		break;		}
	case 1:
		{	m_vecTile[iIndex]->byOption = FAST;			break;		}	
	case 6:
		{	m_vecTile[iIndex]->byOption = DANGEROUS;	break;		}
	case 7:
		{	m_vecTile[iIndex]->byOption = ENDPOINT;		break;		}
	case 8:
		{	m_vecTile[iIndex]->byOption = VERYFAST;		break;		}
	case 11:
	case 12:
	case 13:
	case 14:
		{	m_vecTile[iIndex]->byOption = VERYSLOW;		break;		}
	case 10:
		{	m_vecTile[iIndex]->byOption = SLOW;		break;		}
	case 9:
	case 15:
	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
	case 21:
		{	m_vecTile[iIndex]->byOption = NONPASSALE;		break;		}
	}

	Invalidate(FALSE);
	CScrollView::OnLButtonDown(nFlags, point);
}

bool CMaptoolView::CollisionMouseToTile( D3DXVECTOR2 vMousePos ,const TILE* pTileInfo )
{

	if(	sqrtf(	pow((pTileInfo->vPos.x+4-vMousePos.x),2) + 	pow((pTileInfo->vPos.y+3-vMousePos.y),2)	) <28)
	{
		return true;
	}


	return false;
}


void CMaptoolView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	if(!(GetKeyState(VK_LBUTTON)&0x8000))
		return;

	OnLButtonDown(nFlags,point);


	CScrollView::OnMouseMove(nFlags, point);
}
