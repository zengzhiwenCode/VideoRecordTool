
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
#include "DuiSysInfoDlg.h"
#include "DuiSerialErrorDlg.h"

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
		g_serial.write(msg + "\r\n");
	}
}

enum timer_id {
	preview = 1,
	updatetip,
};



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

void CvrmfcDlg::lang_obs::on_update(const int & lang)
{
	if (dlg) {
		dlg->on_update(lang);
	}
}

void CvrmfcDlg::on_update(const int & lang)
{
	CRect rc;
	GetWindowRect(rc);
	if (lang == 1) {
		rc.left = rc.right - 270;
	} else {
		rc.left = rc.right - 226;
	}
	rc.bottom = rc.top + 22;

	tip_->SetWindowPos(&wndTopMost, rc.left, rc.top, rc.Width(), rc.Height(), SWP_SHOWWINDOW);
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
	ON_MESSAGE(WM_REFRESH_MAT, &CvrmfcDlg::OnRefreshMat)
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()


// CvrmfcDlg message handlers
void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	HWND hWnd = (HWND)(userdata);
	if (event == EVENT_LBUTTONUP) {
		::PostMessageW(hWnd, WM_LBUTTONDOWN,0,0);
		::PostMessageW(hWnd, WM_LBUTTONUP,0,0);
	} else if (event == EVENT_MOUSEMOVE) {


	}
}
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
	m_bkbrush.CreateSolidBrush(RGB(0x59, 0x59, 0x59));

	{
		cvNamedWindow(PLAYER_NAME, 0);
		HWND hWnd = (HWND)cvGetWindowHandle(PLAYER_NAME);
		::ShowWindow(hWnd, SW_HIDE);
		HWND hParent = ::GetParent(hWnd);
		::ShowWindow(hParent, SW_HIDE);
		setMouseCallback(PLAYER_NAME, CallBackFunc, m_hWnd);
		pcvwnd_ = CWnd::FromHandle(hWnd);
		pcvwnd_->ModifyStyle(WS_CAPTION | WS_SYSMENU, 0);
		//pcvwnd_->ModifyStyleEx(0, WS_EX_CLIENTEDGE);
	}

	// init rc
	{
		range_log("init rc");
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
		SetWindowPos(&wndTop, x, y, cx, cy, SWP_SHOWWINDOW);
#endif	
	}
	
	auto cfg = config::get_instance();

	// init tip
	{
		range_log("init tip");
		CRect rc;
		GetWindowRect(rc);
		if (cfg->get_lang() == "en") {
			rc.left = rc.right - 270;
		} else {
			rc.left = rc.right - 226;
		}
		rc.bottom = rc.top + 22;

		tip_ = std::make_shared<CAlarmTextDlg>(this);
		tip_->Create(IDD_DIALOG_ALARM_TEXT);
		//ScreenToClient(rc);
		tip_->SetWindowPos(&wndTopMost, rc.left, rc.top, rc.Width(), rc.Height(), SWP_SHOWWINDOW);
		CString txt;
		usb_storage_plugin_ = !config::list_removable_drives().empty();
		txt.Format(L"%s %s", 
				   now_to_wstring().c_str(), 
				  /* tr(IDS_STRING_BRIGHTNESS), 
				   brightness_level_, */
				   usb_storage_plugin_ ? tr(IDS_STRING_U_IN) : tr(IDS_STRING_U_OUT));
		tip_->SetText(txt);
		tip_->Show();

		obs_ = std::make_shared<lang_obs>();
		obs_->dlg = this;
		cfg->register_observer(obs_);
	}

	// init second tip
	{
		range_log("init second tip");
		CRect rc;
		GetWindowRect(rc);
		rc.left = rc.right - 120;
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
		range_log("init bottom tool");
		CRect rc;
		GetWindowRect(rc);
		rc.top = rc.bottom - 65;
		//rc.bottom -= 5;
		//rc.left += 125;
		//rc.right -= 125;

		CPaintManagerUI::SetInstance(AfxGetInstanceHandle());                    // 指定duilib的实例
		CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + L"\\skin");    // 指定duilib资源的路径，这里指定为和exe同目录
		dui_bt_ = std::make_shared<CDuiBottomTool>(L"bottomtool.xml");
		dui_bt_->brightness_level_ = brightness_level_;
		cfg->register_observer(dui_bt_);
		dui_bt_->Create(m_hWnd, L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
		::SetWindowPos(dui_bt_->GetHWND(), HWND_TOPMOST, rc.left, rc.top, rc.Width(), rc.Height(), SWP_SHOWWINDOW);
		GetWindowRect(rc);
		dui_bt_->set_rc_maindlg(rc);
		dui_bt_->set_mode(CDuiBottomTool::mode::mainwnd);
		dui_bt_->ShowWindow(false, false);
	}
	
	// init serial
	do{
		range_log("init serial");
		auto ports = serial::list_ports();
		if (ports.empty()) {
			CDuiSerialErrorDlg::show_error(m_hWnd, trw(IDS_STRING_NO_COM));
			//MessageBox(tr(IDS_STRING_NO_COM), tr(IDS_STRING_ERROR), MB_ICONERROR);
			//ExitProcess(0);
			//return 1;
			break;
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
			//MessageBox(tr(IDS_STRING_COM_ERR), tr(IDS_STRING_ERROR), MB_ICONERROR);
			CDuiSerialErrorDlg::show_error(m_hWnd, trw(IDS_STRING_COM_ERR));
			//ExitProcess(0);
			//return 1;
		}
	} while (false);

	int fps = 0;
	// init video
	{
		range_log("init video");
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

		//bool need_adjust_video_or_camera = false;
		mi mi = {};
		procamp vamp = {};
		camera_set cam = {};
		if (dscap_.get_info(cfg->get_vidx(), mi, vamp, cam)) {
			cfg->set_mi(mi);
			cfg->set_procamp(vamp);
			cfg->set_camera(cam);

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

			fps = mi[cfg->get_vtype()].fps;
		}

		if (dscap_.OpenCamera(cfg->get_vidx(), false, cfg->get_video_w(), cfg->get_video_h(), cfg->get_vtype().c_str())) {
			cfg->set_video_w(dscap_.GetWidth());
			cfg->set_video_h(dscap_.GetHeight());

			adjust_player_size(cfg->get_video_w(), cfg->get_video_h());

			fps_.begin = std::chrono::steady_clock::now();
			fps_.frames = 0;
		} else {
			JLOG_ERRO("open camera failed");
		}
	}

	recalc_fps();
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
//#ifdef _DEBUG
#ifdef USE_THREAD_TO_CAP_MAT
	stop_worker();
#endif
	CDialogEx::OnCancel();
//#endif // !_DEBUG
}

int ShowMat(cv::Mat img, HWND hWndDisplay)
{
	if (img.channels()<3) {
		return -1;
	}

	RECT rect;
	GetClientRect(hWndDisplay, &rect);
	cv::Mat imgShow(abs(rect.top - rect.bottom), abs(rect.right - rect.left), CV_8UC3);
	resize(img, imgShow, imgShow.size());

	ATL::CImage CI;
	int w = imgShow.cols;//宽  
	int h = imgShow.rows;//高  
	int channels = imgShow.channels();//通道数  

	CI.Create(w, h, 8 * channels);
	uchar *pS;
	uchar *pImg = (uchar *)CI.GetBits();//得到CImage数据区地址  
	int step = CI.GetPitch();
	for (int i = 0; i<h; i++) {
		pS = imgShow.ptr<uchar>(i);
		for (int j = 0; j<w; j++) {
			for (int k = 0; k<3; k++)
				*(pImg + i*step + j * 3 + k) = pS[j * 3 + k];
			//注意到这里的step不用乘以3  
		}
	}

	HDC dc;
	dc = GetDC(hWndDisplay);
	CI.Draw(dc, 0, 0);
	ReleaseDC(hWndDisplay, dc);
	CI.Destroy();

	return 0;
}

void CvrmfcDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent) {
	case timer_id::preview:
	{
		range_log rl("timer_id::preview");
		if (dscap_.isOpened()) {
			auto frame_ = dscap_.QueryFrame();
			if (frame_.data) {
				//drawer_->DrawImg(frame_);
				imshow(PLAYER_NAME, frame_);
				//ShowMat(frame_, m_player.GetSafeHwnd());

				if (record_.recording /*&& record_.writer && record_.writer->isOpened()*/) {
					//record_.writer->write(frame_);
					{
						range_log rl("timer_id::preview save frame");
						std::lock_guard<std::mutex> lg(mutex_);
						recorded_frames_.push_back(frame_.clone());
					}
					cv_.notify_one();
					
					rec_tip_->SetText(utf8::a2w(fps_.get_string() + " " + record_.get_time()).c_str());
					rec_tip_->Invalidate();

					int rec_time = config::get_instance()->get_max_rec_minutes();
					if (rec_time != 0 && record_.get_minutes() >= rec_time) {
						do_stop_record();
					}

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
		txt.Format(L"%s     %s",
				   now_to_wstring().c_str(),
				  /* tr(IDS_STRING_BRIGHTNESS),
				   brightness_level_,*/
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

void CvrmfcDlg::worker()
{
	while (record_.recording) {
		std::list<cv::Mat> frames;

		{
			std::unique_lock<std::mutex> ul(mutex_);
			if (cv_.wait_for(ul, std::chrono::milliseconds(1), [this] {return !record_.recording || !recorded_frames_.empty(); })) {
				std::copy(recorded_frames_.begin(), recorded_frames_.end(), std::back_inserter(frames));
				recorded_frames_.clear();
			}
		}
		
		for (auto frame : frames) {
			record_.writer->write(frame);
		}
	}
	
}

#ifdef USE_THREAD_TO_CAP_MAT
void CvrmfcDlg::stop_worker(bool close_cam)
{
	AUTO_LOG_FUNCTION;
	{
		std::lock_guard<std::mutex> lg(mutex_);
		running_ = false;
	}

	cv_.notify_one();

	//std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms_ * 5));
	if (thread_.joinable()) {
		thread_.join();
	}

	if (close_cam && dscap_.isOpened()) {
		dscap_.CloseCamera();
	}
}

void CvrmfcDlg::start_worker()
{
	AUTO_LOG_FUNCTION;
	running_ = true;
	thread_ = std::thread(&CvrmfcDlg::worker, this);
}


{
	AUTO_LOG_FUNCTION;
	while (running_) {
		std::unique_lock<std::mutex> ul(mutex_);
		cv_.wait_for(ul, std::chrono::milliseconds(sleep_ms_), [this]() {return !running_; });

		if (!running_) {
			break;
		}

		if (dscap_.isOpened()) {
			frame_ = dscap_.QueryFrame();
			if (frame_.data) {
				SendMessage(WM_REFRESH_MAT);
			}
		}
	}
}
#endif

afx_msg LRESULT CvrmfcDlg::OnRefreshMat(WPARAM wParam, LPARAM lParam)
{
	if (running_) {
		draw_mat();
	}

	return 0;
}

void CvrmfcDlg::draw_mat()
{
	/*drawer_->DrawImg(frame_);

	if (record_.recording && record_.writer && record_.writer->isOpened()) {
		record_.writer->write(frame_);
		rec_tip_->SetText(utf8::a2w(fps_.get_string() + " " + record_.get_time()).c_str());
		rec_tip_->Invalidate();

		int rec_time = config::get_instance()->get_max_rec_minutes();
		if (rec_time != 0 && record_.get_minutes() >= rec_time) {
			do_stop_record();
		}

	} else {
		rec_tip_->SetText(utf8::a2w(fps_.get_string()).c_str());
		rec_tip_->Invalidate();
	}*/
}

void CvrmfcDlg::adjust_player_size(int w, int h)
{
	w--;
	h--;
	CRect rc, rcplayer;
	GetClientRect(rc);
	rcplayer = rc;
	double r = rc.Width() * 1.0 / rc.Height();
	double rr = w * 1.0 / h;
	double width = 1.0 * rc.Width();
	double height = 1.0 * rc.Height();

	if (r <= rr) {
		int hh = static_cast<int>(width / rr);
		int gap = (rc.Height() - hh) / 2;
		rcplayer.top += gap;
		rcplayer.bottom -= gap;
	} else {
		int ww = static_cast<int>(height * rr);
		int gap = (rc.Width() - ww) / 2;
		rcplayer.left += gap;
		rcplayer.right -= gap;
	}

	//ScreenToClient(rcplayer);
	m_player.MoveWindow(rcplayer);
	//drawer_ = std::make_shared<PkMatToGDI>(m_player.GetSafeHwnd(), false);
	
	//cvNamedWindow(PLAYER_NAME, 0);
	//cvDestroyWindow(PLAYER_NAME);
	cvResizeWindow(PLAYER_NAME, rcplayer.Width()/* - (rcplayer.Width() % 4)*/, rcplayer.Height()/* - (rcplayer.Width() % 4)*/);
	HWND hWnd = (HWND)cvGetWindowHandle(PLAYER_NAME);
	::ShowWindow(hWnd, SW_HIDE);
	
	//cvResizeWindow(PLAYER_NAME, w, h);
	::SetParent(hWnd, m_player.GetSafeHwnd());
	::ShowWindow(hWnd, SW_SHOW);

	//CWnd* pwnd = CWnd::FromHandle(hWnd);
	
}

HBRUSH CvrmfcDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	return m_bkbrush;
}

void CvrmfcDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	
	serial_send("off");

	JLOG_INFO("vrmfc stopped running");
}

afx_msg LRESULT CvrmfcDlg::OnDeviceChange(WPARAM wParam, LPARAM lParam)
{
	usb_storage_plugin_ = !config::list_removable_drives().empty();
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
			CRect rc;
			::GetWindowRect(dui_bt_->GetHWND(), rc);
			CPoint lt(rc.left, rc.top);
			CPoint rb(rc.right, rc.bottom);
			::ScreenToClient(dui_bt_->GetHWND(), &lt);
			::ScreenToClient(dui_bt_->GetHWND(), &rb);
			rc.left = lt.x;
			rc.top = lt.y;
			rc.bottom = rb.y;
			rc.right = rb.x;
			::InvalidateRect(dui_bt_->GetHWND(), rc, 1);
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
		//const auto prefix = "printf(\""; const size_t szpre = 8;
		const auto postfix = "\r\n"; const size_t szpost = 2;
		//auto pos1 = com_data_.find(prefix);
		auto pos2 = com_data_.find(postfix);
		while (pos2 != npos && 0 < pos2) {
			auto data = com_data_.substr(0, pos2);
			com_data_ = com_data_.substr(pos2 + szpost);
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

void previewr()
{
	
}

void CvrmfcDlg::recalc_fps()
{
	auto cfg = config::get_instance();

#ifdef USE_THREAD_TO_CAP_MAT
	stop_worker(false);
#else
	KillTimer(timer_id::preview);
#endif

	int fps = cfg->get_mi()[cfg->get_vtype()].fps;

	int gap = fps == 0 ? 30 : 1000 / fps;
	gap -= 10;
	if (cfg->get_vtype() == MT_MJPG) {
		gap = 30;
	}

	//gap -= 5;
	fps_.begin = std::chrono::steady_clock::now();
	fps_.frames = 0;

#ifdef USE_THREAD_TO_CAP_MAT
	sleep_ms_ = gap;
	start_worker();
#else
	SetTimer(timer_id::preview, gap, nullptr);

#endif	
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

bool CvrmfcDlg::do_record()
{
	AUTO_LOG_FUNCTION; if (modual_dialog_opened_) { return false; }
	JLOG_INFO("recording_={}", record_.recording);
	if (record_.recording) { do_stop_record(); return false; }
	if (!dscap_.isOpened()) { return false; }
	record_.file = config::get_instance()->create_new_video_path();
	//auto cfg = config::get_instance();
	auto fourcc = CV_FOURCC('M', 'J', 'P', 'G'); // todo
	auto width = dscap_.GetWidth();
	auto height = dscap_.GetHeight();
	record_.writer = std::make_shared<cv::VideoWriter>();
	record_.writer->open(record_.file,
						 fourcc,
						 fps_.get(),
						 //cfg->get_mi()[cfg->get_vtype()].fps,
						 cv::Size(width, height),
						 true);
	recorded_frames_.clear();
	record_.recording = true;
	thread_ = std::thread(&CvrmfcDlg::worker, this);
	
	record_.begin = std::chrono::steady_clock::now();
	dui_bt_->enable_btns(false);
	return true;
}

void CvrmfcDlg::do_stop_record()
{
	if (!record_.recording)return;
	record_.recording = false;
	
	if (thread_.joinable()) {
		thread_.join();
	}
	record_.writer->set(cv::CAP_PROP_FPS, fps_.get());
	record_.writer.reset();
	recorded_frames_.clear();
	record_.file.clear();
	fps_.frames = 0;
	fps_.begin = std::chrono::steady_clock::now();
	dui_bt_->enable_btns(true);
	dui_bt_->on_record_stopped();
}

void CvrmfcDlg::do_capture()
{
	AUTO_LOG_FUNCTION; AUTO_LOCK_DLG;
	auto cfile = config::get_instance()->create_new_capture_path();
	
	if (dscap_.isOpened()) {
		Mat img = dscap_.QueryFrame();
		if (img.data) {
			cv::imwrite(cfile, img);

			if (CDuiPreviewCaptureDlg::make_xml(600, 400)) {
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
	rc.bottom -= 65;
#ifdef USE_THREAD_TO_CAP_MAT
	stop_worker(false);
#else
	KillTimer(timer_id::preview);
#endif
	
	auto cfg = config::get_instance();
	cv::Mat mat = cv::Mat::zeros(cfg->get_video_w(), cfg->get_video_h(), CV_8UC3);
	//drawer_->DrawImg(mat);
	imshow(PLAYER_NAME, mat);
	//ShowMat(mat, m_player.GetSafeHwnd());
	return true;
}

void CvrmfcDlg::do_update_pic_sel(fv pics, fviters iters)
{
	dui_bt_->update_pic_sel(pics, iters);
}

void CvrmfcDlg::do_update_video_sel(fv videos, fviters iters)
{
	dui_bt_->update_video_sel(videos, iters);
}

bool CvrmfcDlg::do_picview_mode_show_or_hide_tools(bool show)
{
	return dui_bt_->show_pic_tip(show);
}

bool CvrmfcDlg::do_video_view_mode_show_or_hide_tools(bool show)
{
	AUTO_LOG_FUNCTION;
	return dui_bt_->show_video_tips(show);
}

bool CvrmfcDlg::do_video_view_mode_pos_changed(const std::wstring & cur, const std::wstring & total, int pos)
{
	AUTO_LOG_FUNCTION;
	return dui_bt_->on_video_pos_changed(cur, total, pos);
}

bool CvrmfcDlg::do_video_view_mode_user_change_pos(int pos)
{
	return dui_bt_->on_user_change_video_pos(pos);
}

void CvrmfcDlg::do_file_manager_over()
{
	UNLOCK_DLG;
	tip_->Show();
	rec_tip_->Show();
	recalc_fps();
}

void CvrmfcDlg::do_settings()
{
	AUTO_LOG_FUNCTION; AUTO_LOCK_DLG;
	auto dlg = std::make_shared<CDuiSettingsDlg>(L"settings.xml");
	config::get_instance()->register_observer(dlg);
	dlg->Create(m_hWnd, L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
	dlg->ShowModal();
}

bool CvrmfcDlg::do_update_capmode(const std::string & mode)
{
	auto cfg = config::get_instance();
#ifdef USE_THREAD_TO_CAP_MAT
	stop_worker();
#else
	KillTimer(timer_id::preview);
	dscap_.CloseCamera();
#endif
	if (dscap_.OpenCamera(cfg->get_vidx(), false, cfg->get_video_w(), cfg->get_video_h(), mode.c_str())) {
		cfg->set_vtype(mode);

		recalc_fps();
		return true;
	}
	return false;
}

bool CvrmfcDlg::do_update_resolution(misz sz)
{
	auto cfg = config::get_instance();
#ifdef USE_THREAD_TO_CAP_MAT
	stop_worker();
#else
	KillTimer(timer_id::preview);
	dscap_.CloseCamera();
#endif
	if (dscap_.OpenCamera(cfg->get_vidx(), false, sz.first, sz.second, cfg->get_vtype().c_str())) {
		cfg->set_video_w(sz.first);
		cfg->set_video_h(sz.second);
		adjust_player_size(sz.first, sz.second);
		recalc_fps();
		return true;
	}
	return false;
}

bool CvrmfcDlg::do_update_video(VideoProcAmpProperty p, int value)
{
	if (dscap_.isOpened() && dscap_.update_video(p, value)) {
		auto cfg = config::get_instance();
		cfg->set_procamp(dscap_.get_video());
		return true;
	}
	return false;
}

bool CvrmfcDlg::do_reset_video()
{
	if (dscap_.isOpened() && dscap_.reset_video()) {
		config::get_instance()->set_procamp(dscap_.get_video());
		recalc_fps();
		return true;
	}
	return false;
}

bool CvrmfcDlg::do_update_camera(CameraControlProperty p, int value)
{
	if (dscap_.isOpened() && dscap_.update_camera(p, value)) {
		config::get_instance()->set_camera(dscap_.get_camera());
		return true;
	}
	return false;
}

bool CvrmfcDlg::do_reset_camera()
{
	if (dscap_.isOpened() && dscap_.reset_camera()) {
		config::get_instance()->set_camera(dscap_.get_camera());
		return true;
	}
	return false;
}

void CvrmfcDlg::do_system_info()
{
	AUTO_LOG_FUNCTION; AUTO_LOCK_DLG;
	CDuiSysInfoDlg dlg(L"sysinfo.xml");
	dlg.Create(m_hWnd, L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
	dlg.ShowModal();
}

void CvrmfcDlg::do_adjust_brightness()
{
	AUTO_LOG_FUNCTION;
	static const char* cmd[] = { "lt0", "lt1", "lt2", "lt3", "lt4","lt5", "lt6" };
	static auto idx = 0;
	serial_send(cmd[idx++]);
	if (idx == 7) {
		idx = 0;
	}

	brightness_level_ = idx;

	dui_bt_->set_brightness_level(idx);
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

int CvrmfcDlg::_record::get_minutes()
{
	auto now = std::chrono::steady_clock::now();
	auto diff = now - begin;
	auto min = std::chrono::duration_cast<std::chrono::minutes>(diff).count();
	return static_cast<int>(min);
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
	/*if (total_sec >= 3) {
		frames = 0;
		begin = now;
	} else */if (total_sec > 0) {
		auto fps = frames / total_sec;
		prev_fps = std::to_string(fps) + "fps ";
	}

	return prev_fps;
}

BOOL CvrmfcDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}
