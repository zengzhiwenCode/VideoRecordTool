
// vrmfcDlg.h : header file
//

#pragma once
#include "afxwin.h"

class CAlarmTextDlg;
class PkMatToGDI;
class CDuiBottomTool;

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
	CStatic m_player;
	std::shared_ptr<PkMatToGDI> drawer_ = {};
	cv::VideoCapture capture_ = {};
	CBrush m_bkbrush = {};
	std::shared_ptr<CAlarmTextDlg> tip_ = {};
	std::shared_ptr<CDuiBottomTool> dui_bt_ = {};
	bool bottom_show_ = false;
	std::string com_data_ = {};
	int brightness_level_ = 2;
	int temperature_ = 0;
	bool usb_storage_plugin_ = false;

	void adjust_player_size(int w, int h);
	void handle_com();
	void process_com(const std::string& cmd);

	// Public interface
public:
	void exit_windows();
	void record();
	void capture();
	void file_manager();
	void settings();
	void system_info();
	void adjust_brightness();


	// Generated message map functions
public:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnDeviceChange(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
