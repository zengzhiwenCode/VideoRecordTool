
// vrmfcDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "AlarmTextDlg.h"

class PkMatToGDI;

// CvrmfcDlg dialog
class CvrmfcDlg : public CDialogEx
{
// Construction
public:
	CvrmfcDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VRMFC_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	std::shared_ptr<PkMatToGDI> drawer_ = {};
	cv::VideoCapture capture_ = {};
	CBrush m_bkbrush = {};
	std::shared_ptr<CAlarmTextDlg> tip = {};
	std::string com_data_ = {};
	int brightness_level_ = 2;
	int temperature_ = 0;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CStatic m_player;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void adjust_player_size(int w, int h);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	void handle_com();
	void process_com(const std::string& cmd);
};
