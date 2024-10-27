
// MaptoolView.h : CMaptoolView 클래스의 인터페이스
//

#pragma once
#include "Defines.h"
#include "MaptoolDoc.h"
class CMaptoolView : public CScrollView
{
protected: // serialization에서만 만들어집니다.
	CMaptoolView();
	DECLARE_DYNCREATE(CMaptoolView)

// 특성입니다.
public:
	CMaptoolDoc* GetDocument() const;

// 작업입니다.
public:
	vector<TILE*>		m_vecTile;
	int					m_iSelDrawID;
public:
	void InitTile();
	bool CollisionMouseToTile(D3DXVECTOR2 vMousePos
		,const TILE* pTileInfo);


// 재정의입니다.
public:
	virtual void OnDraw(CDC* pDC);  // 이 뷰를 그리기 위해 재정의되었습니다.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 구현입니다.
public:
	virtual ~CMaptoolView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 생성된 메시지 맵 함수
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

#ifndef _DEBUG  // MaptoolView.cpp의 디버그 버전
inline CMaptoolDoc* CMaptoolView::GetDocument() const
   { return reinterpret_cast<CMaptoolDoc*>(m_pDocument); }
#endif

