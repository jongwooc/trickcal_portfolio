#pragma once
#include "afxwin.h"


// CPage1 ��ȭ �����Դϴ�.

class CPage1 : public CPropertyPage
{
	DECLARE_DYNAMIC(CPage1)

public:
	CPage1();
	virtual ~CPage1();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_PAGE1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_Combo;
	CStatic m_Picture;

	HBITMAP	 m_BitMap[38];
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeCombo1();

};
