
// vrmfcDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "CameraDS.h"

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
	//cv::VideoCapture capture_ = {};
	CCameraDS dscap_ = {};
	CBrush m_bkbrush = {};
	std::shared_ptr<CAlarmTextDlg> tip_ = {};
	std::shared_ptr<CAlarmTextDlg> rec_tip_ = {};
	std::shared_ptr<CDuiBottomTool> dui_bt_ = {};
	bool bottom_show_ = false;
	std::string com_data_ = {};
	int brightness_level_ = 2;
	int temperature_ = 0;
	bool usb_storage_plugin_ = false;
	

	struct _fps {
		std::chrono::steady_clock::time_point begin = {};
		long long frames = 0;
		std::string prev_fps = {};

		int get();
		std::string get_string();
	} fps_ = {};

	struct _record {
		bool recording = false;
		std::string file = {};
		std::shared_ptr<cv::VideoWriter> writer = {};
		std::chrono::steady_clock::time_point begin = {};
		std::string prev_time_str = {};
		
		std::string get_time();
	} record_ = {};

	void adjust_player_size(int w, int h);
	void handle_com();
	void process_com(const std::string& cmd);


	// Public interface
public:
	void do_exit_windows();
	void do_record();
	void do_stop_record();
	void do_capture();
	bool do_file_manager(CRect& rc);
	void do_view_pic(fv pics, fviter iter);
	void do_play_video(fv videos, fviter iter);
	void do_file_manager_over();
	void do_settings();
	bool do_update_capmode(const std::string& mode);
	void do_system_info();
	void do_adjust_brightness();


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
