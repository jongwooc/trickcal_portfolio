
// MaptoolView.h : CMaptoolView Ŭ������ �������̽�
//

#pragma once
#include "Defines.h"
#include "MaptoolDoc.h"
class CMaptoolView : public CScrollView
{
protected: // serialization������ ��������ϴ�.
	CMaptoolView();
	DECLARE_DYNCREATE(CMaptoolView)

// Ư���Դϴ�.
public:
	CMaptoolDoc* GetDocument() const;

// �۾��Դϴ�.
public:
	vector<TILE*>		m_vecTile;
	int					m_iSelDrawID;
public:
	void InitTile();
	bool CollisionMouseToTile(D3DXVECTOR2 vMousePos
		,const TILE* pTileInfo);


// �������Դϴ�.
public:
	virtual void OnDraw(CDC* pDC);  // �� �並 �׸��� ���� �����ǵǾ����ϴ�.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// �����Դϴ�.
public:
	virtual ~CMaptoolView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ������ �޽��� �� �Լ�
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // MaptoolView.cpp�� ����� ����
inline CMaptoolDoc* CMaptoolView::GetDocument() const
   { return reinterpret_cast<CMaptoolDoc*>(m_pDocument); }
#endif

