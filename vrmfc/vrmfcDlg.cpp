
// vrmfcDlg.cpp : implementation file
//

#include "stdafx.h"
#include "vrmfc.h"
#include "vrmfcDlg.h"
#include "afxdialogex.h"
#include "config.h"
#include <TlHelp32.h>
#include <Dbt.h>

#pragma comment(lib,"shlwapi.lib")

#include "mat2gdi.h"
#include "serial/serial.h"
#include "AlarmTextDlg.h"
#include "DuiBottomTool.h"
#include "DuiConfirmExitDlg.h"
#include "DuiSettingsDlg.h"
#include "DuiPreviewCaptureDlg.h"

#ifdef _DEBUG
#pragma comment(lib, "serial/lib/Debug/serial.lib")
#else
#pragma comment(lib, "serial/lib/Release/serial.lib")
#endif // _DEBUG


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {

serial::Serial g_serial;

void serial_send(const std::string& msg)
{
	if (g_serial.isOpen()) {
		g_serial.write("printf(\"" + msg + "\r\n\")");
	}
}

enum timer_id {
	preview = 1,
	updatetip,
};

std::vector<char> list_removable_drives() {
	std::vector<char> v;
	for (char i = 'A'; i <= 'Z'; i++) {
		char x[3] = { i, ':' };
		UINT Type = GetDriveTypeA(x);
		if (Type == DRIVE_REMOVABLE) {
			v.push_back(i);
		}
	}
	return v;
}

bool modual_dialog_opened_ = false;

#define UNLOCK_DLG do { modual_dialog_opened_ = false; } while(false);

class auto_dlg_lock {
public:
	auto_dlg_lock() {}
	~auto_dlg_lock() {
		UNLOCK_DLG;
	}
};

#define TEST_LOCK_DLG	if (modual_dialog_opened_) return;
#define LOCK_DLG		TEST_LOCK_DLG; modual_dialog_opened_ = true;
#define AUTO_LOCK_DLG	LOCK_DLG; auto_dlg_lock adl;



}

using namespace cv;

// CvrmfcDlg dialog

CvrmfcDlg::CvrmfcDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_VRMFC_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CvrmfcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PLAYER, m_player);
}

BEGIN_MESSAGE_MAP(CvrmfcDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CvrmfcDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CvrmfcDlg::OnBnClickedCancel)
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_DEVICECHANGE, &CvrmfcDlg::OnDeviceChange)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CvrmfcDlg message handlers

BOOL CvrmfcDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL) {
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty()) {
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_bkbrush.CreateSolidBrush(RGB(0, 0, 0));

	// init logger
	{
		auto path = get_exe_path_a() + "/log";
		CreateDirectoryA(path.c_str(), nullptr);
		path += "/vrmfc";
		jlib::init_logger(path);
	}

	// init rc
	{
		int x = 0;
		int y = 0;
		int cx = GetSystemMetrics(SM_CXSCREEN);
		int cy = GetSystemMetrics(SM_CYSCREEN);

#ifdef _DEBUG
		int w = 1024;
		int h = 768;
		x = (cx - w) / 2;
		y = (cy - h) / 2;
		cx = w;
		cy = h;
		SetWindowPos(&wndTop, x, y, cx, cy, SWP_SHOWWINDOW);
#else
		SetWindowPos(&wndTopMost, x, y, cx, cy, SWP_SHOWWINDOW);
#endif	
	}
	
	auto cfg = config::get_instance();

	// init tip
	{
		CRect rc;
		GetWindowRect(rc);
		if (cfg->get_lang() == "en") {
			rc.left = rc.right - 325;
		} else {
			rc.left = rc.right - 255;
		}
		rc.bottom = rc.top + 22;

		tip_ = std::make_shared<CAlarmTextDlg>(this);
		tip_->Create(IDD_DIALOG_ALARM_TEXT);
		//ScreenToClient(rc);
		tip_->SetWindowPos(&wndTopMost, rc.left, rc.top, rc.Width(), rc.Height(), SWP_SHOWWINDOW);
		CString txt;
		usb_storage_plugin_ = !list_removable_drives().empty();
		txt.Format(L"%s %s%d %s", 
				   now_to_wstring().c_str(), 
				   tr(IDS_STRING_BRIGHTNESS), 
				   brightness_level_, 
				   usb_storage_plugin_ ? tr(IDS_STRING_U_IN) : tr(IDS_STRING_U_OUT));
		tip_->SetText(txt);
		tip_->Show();
	}

	// init second tip
	{
		CRect rc;
		GetWindowRect(rc);
		rc.left = rc.right - 150;
		rc.top += 25;
		rc.bottom = rc.top + 22;

		rec_tip_ = std::make_shared<CAlarmTextDlg>(this);
		rec_tip_->Create(IDD_DIALOG_ALARM_TEXT);
		rec_tip_->SetWindowPos(&wndTopMost, rc.left, rc.top, rc.Width(), rc.Height(), SWP_SHOWWINDOW);
		rec_tip_->SetText(L"0fps");
		rec_tip_->Show();
	}

	// init bottom tool
	{
		CRect rc;
		GetWindowRect(rc);
		rc.top = rc.bottom - 85;
		rc.bottom -= 5;
		rc.left += 5;
		rc.right -= 5;

		CPaintManagerUI::SetInstance(AfxGetInstanceHandle());                    // 指定duilib的实例
		CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + L"\\skin");    // 指定duilib资源的路径，这里指定为和exe同目录
		dui_bt_ = std::make_shared<CDuiBottomTool>(L"bottomtool.xml");
		dui_bt_->Create(m_hWnd, L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
		::MoveWindow(dui_bt_->GetHWND(), rc.left, rc.top, rc.Width(), rc.Height(), 0);
		dui_bt_->set_mode(CDuiBottomTool::mode::mainwnd);
		dui_bt_->ShowWindow(false, false);
	}
	
	// init serial
	{
		auto ports = serial::list_ports();
		if (ports.empty()) {
			MessageBox(tr(IDS_STRING_NO_COM), tr(IDS_STRING_ERROR), MB_ICONERROR);
			ExitProcess(0);
			return 1;
		}

		if (g_serial.isOpen()) {
			g_serial.close();
		}

		com_data_.clear();

		if (cfg->get_baudrate() == 0) {
			cfg->set_baudrate(9600);
		}

		g_serial.setBaudrate(cfg->get_baudrate());
		g_serial.setTimeout(serial::Timeout::max(), 1, 0, 1, 0);

		std::string port = cfg->get_port();
		auto iter = ports.begin();
		while (true) {
			try {
				g_serial.setPort(port);
				g_serial.open();
				if (g_serial.isOpen()) {
					serial_send("on");
					cfg->set_port(g_serial.getPort());
					break;
				}
			} catch (std::exception& e) {
				JLOG_ERRO(e.what());
				if (iter == ports.end()) {
					break;
				}
				port = iter->port;
				iter++;
			}
		}
		
		if (!g_serial.isOpen()) {
			MessageBox(tr(IDS_STRING_COM_ERR), tr(IDS_STRING_ERROR), MB_ICONERROR);
			ExitProcess(0);
			return 1;
		}
	}

	// init video
	{
		//if (!capture_.open(CV_CAP_DSHOW  + cfg->get_vidx())) {
		//	if (capture_.open(0)) {
		//		cfg->set_vidx(0);
		//	}
		//}

		//if (capture_.isOpened()) {
		//	if (cfg->get_video_w() == 0 || cfg->get_video_h() == 0) {
		//		cfg->set_video_w(static_cast<int>(capture_.get(CAP_PROP_FRAME_WIDTH)));
		//		cfg->set_video_h(static_cast<int>(capture_.get(CAP_PROP_FRAME_HEIGHT)));
		//	} else {
		//		capture_.set(CAP_PROP_FRAME_WIDTH, cfg->get_video_w());
		//		capture_.set(CAP_PROP_FRAME_HEIGHT, cfg->get_video_h());
		//	}

		//	//Mat frame;
		//	//capture_.read(frame);
		//	//capture_.set(CAP_PROP_FOURCC, CV_FOURCC('M', 'P', 'E', 'G'));
		//	//auto f = capture_.get(CAP_PROP_FORMAT);
		//	
		//	auto get_fourcc = [this]() -> std::string {
		//		int ex = static_cast<int>(capture_.get(CV_CAP_PROP_FOURCC));
		//		char EXT[] = { (char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0 };
		//		return EXT;
		//	};

		//	//
		//	//
		//	bool ok = false;
		//	
		//	ok = capture_.set(CAP_PROP_MODE, CAP_MODE_BGR);
		//	JLOG_INFO("CAP_MODE_BGR {}", ok);

		//	ok = capture_.set(CAP_PROP_MODE, CAP_MODE_RGB);
		//	JLOG_INFO("CAP_MODE_RGB {}", ok);

		//	ok = capture_.set(CAP_PROP_MODE, CAP_MODE_YUYV);
		//	JLOG_INFO("CAP_MODE_YUYV {}", ok);

		//	ok = capture_.set(CAP_PROP_CONVERT_RGB, false);
		//	JLOG_INFO("CAP_PROP_CONVERT_RGB {}", ok);

		//	//ok = capture_.set(CAP_PROP_MODE, CAP_MODE_YUYV); 
		//	//JLOG_INFO("yuyv {}", ok);
		//	//JLOG_INFO("fourcc={}", get_fourcc());

		//	ok = capture_.set(CAP_PROP_FOURCC, CV_FOURCC('M', 'J', 'P', 'G'));
		//	JLOG_INFO("MJPG {}", ok);
		//	JLOG_INFO("fourcc={}", get_fourcc());

		//	ok = capture_.set(CAP_PROP_FOURCC, CV_FOURCC('Y', 'U', 'Y', '2')); 
		//	JLOG_INFO("YUY2 {}", ok);
		//	JLOG_INFO("fourcc={}", get_fourcc());
		//	//ok = capture_.set(CAP_PROP_MODE, cap_mode_);

		//	//auto mode = capture_.get(CAP_PROP_MODE);

		//	adjust_player_size(cfg->get_video_w(), cfg->get_video_h());
		//	drawer_ = std::make_shared<PkMatToGDI>(m_player.GetSafeHwnd(), false);		

		//	fps_.begin = std::chrono::steady_clock::now();
		//	fps_.frames = 0;
		//}

		//int w = 960; int h = 720;
		Mat frame;

		//if (cfg->get_video_w() != 0 && cfg->get_video_h() != 0) {
		//	w = cfg->get_video_w();
		//	h = cfg->get_video_h();
		//}

		mi mi;
		if (dscap_.get_mi(cfg->get_vidx(), mi)) {
			cfg->set_mi(mi);
			auto storaged_type = cfg->get_vtype();
			auto iter = mi.find(storaged_type);
			if (iter == mi.end()) {
				cfg->set_vtype(mi.begin()->first);
				cfg->set_video_w(mi.begin()->second.default_sz.first);
				cfg->set_video_h(mi.begin()->second.default_sz.second);
			} else {
				bool sz_found = false;
				for (auto sz : iter->second.sizes) {
					if (sz.first == cfg->get_video_w() && sz.second == cfg->get_video_h()) {
						sz_found = true;
						break;
					}
				}

				if (!sz_found) {
					cfg->set_video_w(mi.begin()->second.default_sz.first);
					cfg->set_video_h(mi.begin()->second.default_sz.second);
				}
			}
		}

		if (dscap_.OpenCamera(cfg->get_vidx(), false, cfg->get_video_w(), cfg->get_video_h(), cfg->get_vtype().c_str())) {
			cfg->set_video_w(dscap_.GetWidth());
			cfg->set_video_h(dscap_.GetHeight());

			adjust_player_size(cfg->get_video_w(), cfg->get_video_h());
			drawer_ = std::make_shared<PkMatToGDI>(m_player.GetSafeHwnd(), false);		

			fps_.begin = std::chrono::steady_clock::now();
			fps_.frames = 0;
		}
	}

	SetTimer(timer_id::preview, 30, nullptr);
	SetTimer(timer_id::updatetip, 1000, nullptr);


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CvrmfcDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
	} else {
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CvrmfcDlg::OnPaint()
{
	if (IsIconic()) {
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	} else {
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CvrmfcDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CvrmfcDlg::OnBnClickedOk()
{
	//CDialogEx::OnOK();
}

void CvrmfcDlg::OnBnClickedCancel()
{
#ifdef _DEBUG
	CDialogEx::OnCancel();
#endif // !_DEBUG
}

void CvrmfcDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent) {
	case timer_id::preview:
	{
		if (dscap_.isOpened()) {
			Mat frame = dscap_.QueryFrame();
			if (frame.data) {
				drawer_->DrawImg(frame);

				if (record_.recording && record_.writer && record_.writer->isOpened()) {
					record_.writer->write(frame);
					rec_tip_->SetText(utf8::a2w(fps_.get_string() + " " + record_.get_time()).c_str());
					rec_tip_->Invalidate();
				} else {
					rec_tip_->SetText(utf8::a2w(fps_.get_string()).c_str());
					rec_tip_->Invalidate();
				}
			}
		}
	}
		break;

	case timer_id::updatetip:
	{
		//range_log rl("timer_id::updatetip");
		handle_com();
		CString txt;
		txt.Format(L"%s %s%d %s",
				   now_to_wstring().c_str(),
				   tr(IDS_STRING_BRIGHTNESS),
				   brightness_level_,
				   usb_storage_plugin_ ? tr(IDS_STRING_U_IN) : tr(IDS_STRING_U_OUT));
		tip_->SetText(txt);
		tip_->Invalidate();
	}
		break;

	default:
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CvrmfcDlg::adjust_player_size(int w, int h)
{
	CRect rc, rcplayer;
	GetWindowRect(rc);
	rcplayer = rc;
	double r = rc.Width() * 1.0 / rc.Height();
	double rr = w * 1.0 / h;

	if (r <= rr) {
		int hh = h * rc.Width() / w;
		int gap = (rc.Height() - hh) / 2;
		rcplayer.top += gap;
		rcplayer.bottom -= gap;
	} else {
		int ww = w * rc.Height() / h;
		int gap = (rc.Width() - ww) / 2;
		rcplayer.left += gap;
		rcplayer.right -= gap;
	}

	ScreenToClient(rcplayer);
	m_player.MoveWindow(rcplayer);
}

HBRUSH CvrmfcDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	return m_bkbrush;
}

void CvrmfcDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	serial_send("off");
}

afx_msg LRESULT CvrmfcDlg::OnDeviceChange(WPARAM wParam, LPARAM lParam)
{
	usb_storage_plugin_ = !list_removable_drives().empty();
	return LRESULT();
}

void CvrmfcDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	do {
		if (bottom_show_) {
			if (dui_bt_->get_mode() != CDuiBottomTool::mode::mainwnd) {
				break;
			}

			dui_bt_->ShowWindow(false, false);
		} else {
			dui_bt_->ShowWindow(true, true);
		}

		bottom_show_ = !bottom_show_;
	} while (false);

	CDialogEx::OnLButtonDown(nFlags, point);
}

void CvrmfcDlg::handle_com()
{
	if (!g_serial.isOpen()) {
		return;
	}

	com_data_ = g_serial.read(1024);
	if (!com_data_.empty()) {
		const auto npos = std::string::npos;
		const auto prefix = "printf(\""; const size_t szpre = 8;
		const auto postfix = "\r\n\")"; const size_t szpost = 4;
		auto pos1 = com_data_.find(prefix);
		auto pos2 = com_data_.find(postfix);
		while (pos1 != npos && pos2 != npos && pos1 < pos2 && pos1 + szpre < pos2) {
			auto data = com_data_.substr(pos1 + szpre, pos2 - pos1 - szpre);
			com_data_ = com_data_.substr(pos2 + szpost);
			pos1 = com_data_.find(prefix);
			pos2 = com_data_.find(postfix);

			process_com(data);
		}

		com_data_.clear();
	}
}

void CvrmfcDlg::process_com(const std::string & data)
{
	//AUTO_LOG_FUNCTION;
	// process data
#define com_if(var) if (data == (var)) 
#define com_elif(var) else com_if((var))

	com_if ("off") {
		do_exit_windows();
		return;
	} com_elif ("sr0") {
		do_capture();
		return;
	} com_elif("rec") {
		do_record();
		return;
	}

	// bright
	if (std::string::npos != data.find("lt")) {
		for (int i = 0; i < 6; i++) {
			com_if("lt" + std::to_string(i)) {
				brightness_level_ = i;
				return;
			}
		}
	}

	// temperature
	if (std::string::npos != data.find("tem")) {
		std::stringstream ss;
		for (int i = 0; i < 99; i++) {
			ss.clear(); ss.str("");
			ss << "tem" << std::setw(2) << std::setfill('0') << i;
			com_if(ss.str()) {
				temperature_ = i;
				return;
			}
		}
	}
}

void CvrmfcDlg::do_exit_windows()
{
	AUTO_LOG_FUNCTION;
	AUTO_LOCK_DLG;
	CDuiConfirmExitDlg dlg(L"confirm_exit.xml");
	dlg.Create(m_hWnd, L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
	dlg.ShowModal();
	if (dlg.confirmed_) {
		PostMessage(WM_CLOSE);
	}
}

void CvrmfcDlg::do_record()
{
	AUTO_LOG_FUNCTION; TEST_LOCK_DLG;
	JLOG_INFO("recording_={}", record_.recording);
	if (record_.recording) { do_stop_record(); return; }
	if (!dscap_.isOpened()) { return; }
	record_.file = config::get_instance()->create_new_video_path();
	//auto width = static_cast<int>(capture_.get(CAP_PROP_FRAME_WIDTH));
	//auto height = static_cast<int>(capture_.get(CAP_PROP_FRAME_HEIGHT));
	auto width = dscap_.GetWidth();
	auto height = dscap_.GetHeight();
	//int ex = static_cast<int>(capture_.get(CV_CAP_PROP_FOURCC));
	//char ext[] = { (char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0 };
	//auto fourcc = CV_FOURCC(ext[0], ext[1], ext[2], ext[3]);
	// auto fourcc = CV_FOURCC('P', 'I', 'M', '1');
	auto fourcc = CV_FOURCC('M', 'J', 'P', 'G'); // todo
	record_.writer = std::make_shared<cv::VideoWriter>();
	record_.recording = record_.writer->open(record_.file,
											 fourcc /*ex*/ /*CV_FOURCC('P', 'I', 'M', '1')*/, // pim1 for avi
											 /*fps_*/ fps_.get(),
											 cv::Size(width, height),
											 true);
	
	record_.begin = std::chrono::steady_clock::now();
	dui_bt_->enable_btns(false);
}

void CvrmfcDlg::do_stop_record()
{
	if (!record_.recording)return;
	record_.recording = false;
	record_.writer.reset();
	record_.file.clear();
	fps_.frames = 0;
	fps_.begin = std::chrono::steady_clock::now();
	dui_bt_->enable_btns(true);
}

void CvrmfcDlg::do_capture()
{
	AUTO_LOG_FUNCTION; AUTO_LOCK_DLG;
	auto cfile = config::get_instance()->create_new_capture_path();
	
	if (dscap_.isOpened()) {
		Mat img = dscap_.QueryFrame();
		if (img.data) {
			cv::imwrite(cfile, img);

			if (CDuiPreviewCaptureDlg::make_xml(img.cols, img.rows)) {
				CDuiPreviewCaptureDlg dlg(L"capture.xml");
				dlg.set_auto_close();
				dlg.set_image(cfile);
				dlg.Create(m_hWnd, L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
				dlg.ShowModal();
			}
		}
	}
}

bool CvrmfcDlg::do_file_manager(CRect& rc)
{
	AUTO_LOG_FUNCTION; 
	if (modual_dialog_opened_) {
		return false;
	}
	modual_dialog_opened_ = true;

	tip_->Hide();
	rec_tip_->Hide();
	GetWindowRect(rc);
	rc.bottom -= 100;

	return true;
}

void CvrmfcDlg::do_view_pic(fv pics, fviter iter)
{
	dui_bt_->view_pic(pics, iter);
}

void CvrmfcDlg::do_play_video(fv videos, fviter iter)
{
	dui_bt_->play_video(videos, iter);
}

void CvrmfcDlg::do_file_manager_over()
{
	UNLOCK_DLG;
	tip_->Show();
	rec_tip_->Show();
}

void CvrmfcDlg::do_settings()
{
	AUTO_LOG_FUNCTION; AUTO_LOCK_DLG;
	CDuiSettingsDlg dlg(L"settings.xml");
	dlg.Create(m_hWnd, L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
	dlg.ShowModal();
}

void CvrmfcDlg::do_system_info()
{
	AUTO_LOG_FUNCTION;
}

void CvrmfcDlg::do_adjust_brightness()
{
	AUTO_LOG_FUNCTION;
}

std::string CvrmfcDlg::_record::get_time()
{
	auto now = std::chrono::steady_clock::now();
	auto diff = now - begin;

	auto sec = std::chrono::duration_cast<std::chrono::seconds>(diff).count() % 60;
	if (sec < 1) {
		return prev_time_str;
	}

	auto day = std::chrono::duration_cast<std::chrono::hours>(diff).count() / 24;
	auto hour = std::chrono::duration_cast<std::chrono::hours>(diff).count() % 24;
	auto min = std::chrono::duration_cast<std::chrono::minutes>(diff).count() % 60;
	
	std::stringstream ss;
	//ss << std::setw(2) << std::setfill('0');

	if (day > 0) {
		ss << std::setw(2) << std::setfill('0') << day << ":";
	}

	if (hour > 0) {
		ss << std::setw(2) << std::setfill('0') << hour << ":";
	}

	ss << std::setw(2) << std::setfill('0') << min << ":" << std::setw(2) << std::setfill('0') << sec;

	prev_time_str = ss.str();
	return prev_time_str;;
}

int CvrmfcDlg::_fps::get()
{
	auto now = std::chrono::steady_clock::now();
	auto diff = now - begin;

	auto total_sec = std::chrono::duration_cast<std::chrono::seconds>(diff).count();
	if (total_sec > 0) {
		return static_cast<int>(frames / total_sec);
	}

	return 0;
}

std::string CvrmfcDlg::_fps::get_string()
{
	frames++;
	auto now = std::chrono::steady_clock::now();
	auto diff = now - begin;

	auto total_sec = std::chrono::duration_cast<std::chrono::seconds>(diff).count();
	if (total_sec > 0) {
		auto fps = frames / total_sec;
		prev_fps = std::to_string(fps) + "fps ";
	} 

	return prev_fps;
}
