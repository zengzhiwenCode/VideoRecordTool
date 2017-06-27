
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

	class lang_obs : public dp::observer<int>
	{
	public:
		virtual void on_update(const int& lang);
		CvrmfcDlg* dlg;
	};

	std::shared_ptr<lang_obs> obs_ = {};
	
	void on_update(const int & lang);

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
		int get_minutes();
	} record_ = {};

	void adjust_player_size(int w, int h);
	void handle_com();
	void process_com(const std::string& cmd);
	void recalc_fps();

	// Public interface
public:
	void do_exit_windows();
	// return true for recording, false for stopped
	bool do_record();
	void do_stop_record();
	void do_capture();
	bool do_file_manager(CRect& rc);
	void do_update_pic_sel(fv pics, fviters iters);
	void do_update_video_sel(fv videos, fviters iters);
	bool do_picview_mode_show_or_hide_tools(bool show);
	bool do_video_view_mode_show_or_hide_tools(bool show);
	bool do_video_view_mode_pos_changed(const std::wstring& cur, const std::wstring& total, int pos);
	bool do_video_view_mode_user_change_pos(int pos);
	void do_file_manager_over();
	void do_settings();
	bool do_update_capmode(const std::string& mode);
	bool do_update_resolution(misz sz);
	bool do_update_video(VideoProcAmpProperty p, int value);
	bool do_reset_video();
	bool do_update_camera(CameraControlProperty p, int value);
	bool do_reset_camera();
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
