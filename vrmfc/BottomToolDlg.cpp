// BottomToolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "vrmfc.h"
#include "BottomToolDlg.h"
#include "afxdialogex.h"
#include "vrmfcDlg.h"


// CBottomToolDlg dialog

IMPLEMENT_DYNAMIC(CBottomToolDlg, CDialogEx)

CBottomToolDlg::CBottomToolDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_BOTTOM_TOOL, pParent)
{

}

CBottomToolDlg::~CBottomToolDlg()
{
}

void CBottomToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON1, m_btn_exitwin);
}


BEGIN_MESSAGE_MAP(CBottomToolDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CBottomToolDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CBottomToolDlg::OnBnClickedCancel)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CBottomToolDlg message handlers


void CBottomToolDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	//CDialogEx::OnOK();
}


void CBottomToolDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	//CDialogEx::OnCancel();
}


BOOL CBottomToolDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_bkbrush.CreateSolidBrush(RGB(237, 125, 49));
	m_btn_exitwin.SetFaceColor(RGB(31, 78, 321));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


HBRUSH CBottomToolDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	return m_bkbrush;
}
