#pragma once


// CBtn

class CBtn : public CButton
{
	DECLARE_DYNAMIC(CBtn)

public:
	CBtn();
	virtual ~CBtn();

protected:
	DECLARE_MESSAGE_MAP()
	CBrush m_bk;
public:
	afx_msg HBRUSH CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};


