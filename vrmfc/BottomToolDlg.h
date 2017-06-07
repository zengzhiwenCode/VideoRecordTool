#pragma once
#include "BtnST.h"
#include "afxwin.h"
#include "Btn.h"

using gui::control::CButtonST;

// CBottomToolDlg dialog

class CBottomToolDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CBottomToolDlg)

public:
	CBottomToolDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBottomToolDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_BOTTOM_TOOL };
#endif

protected:
	CBrush m_bkbrush = {};
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CMFCButton m_btn_exitwin;
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
