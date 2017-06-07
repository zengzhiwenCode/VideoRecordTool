// Btn.cpp : implementation file
//

#include "stdafx.h"
#include "vrmfc.h"
#include "Btn.h"


// CBtn

IMPLEMENT_DYNAMIC(CBtn, CButton)

CBtn::CBtn()
{

}

CBtn::~CBtn()
{
}


BEGIN_MESSAGE_MAP(CBtn, CButton)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_CREATE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()



// CBtn message handlers




HBRUSH CBtn::CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/)
{
	return m_bk;
}


int CBtn::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CButton::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_bk.CreateSolidBrush(RGB(31, 78, 321));

	return 0;
}


HBRUSH CBtn::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	return m_bk;
}
