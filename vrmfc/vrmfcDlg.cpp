
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
		m_bkbrush.CreateSolidBrush(RGB(0, 0, 0));
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

	// init bottom tool
	{
		CRect rc;
		GetWindowRect(rc);
		rc.top = rc.bottom - 85;
		rc.bottom -= 5;
		rc.left += 25;
		rc.right -= 25;

		CPaintManagerUI::SetInstance(AfxGetInstanceHandle());                    // 指定duilib的实例
		CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + L"\\skin");    // 指定duilib资源的路径，这里指定为和exe同目录
		dui_bt_ = std::make_shared<CDuiBottomTool>(L"bottomtool.xml");
		dui_bt_->Create(m_hWnd, L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
		::MoveWindow(dui_bt_->GetHWND(), rc.left, rc.top, rc.Width(), rc.Height(), 0);
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
		if (!capture_.open(cfg->get_vidx())) {
			if (capture_.open(0)) {
				cfg->set_vidx(0);
			}
		}

		if (capture_.isOpened()) {
			if (cfg->get_video_w() == 0 || cfg->get_video_h() == 0) {
				cfg->set_video_w(static_cast<int>(capture_.get(CAP_PROP_FRAME_WIDTH)));
				cfg->set_video_h(static_cast<int>(capture_.get(CAP_PROP_FRAME_HEIGHT)));
			} else {
				capture_.set(CAP_PROP_FRAME_WIDTH, cfg->get_video_w());
				capture_.set(CAP_PROP_FRAME_HEIGHT, cfg->get_video_h());
			}

			//Mat frame;
			//capture_.read(frame);
			//capture_.set(CAP_PROP_FOURCC, CV_FOURCC('M', 'P', 'E', 'G'));
			//auto f = capture_.get(CAP_PROP_FORMAT);
			//int ex = static_cast<int>(capture_.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form

			//																   // Transform from int to char via Bitwise operators
			//char EXT[] = { (char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0 };

			//
			//
			//capture_.set(CAP_PROP_MODE, CAP_MODE_BGR);
			//capture_.set(CAP_PROP_MODE, CAP_MODE_YUYV);
			//auto mode = capture_.get(CAP_PROP_MODE);

			adjust_player_size(cfg->get_video_w(), cfg->get_video_h());
			drawer_ = std::make_shared<PkMatToGDI>(m_player.GetSafeHwnd(), false);			
		}
	}

	SetTimer(timer_id::preview, 1, nullptr);
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
		Mat frame;
		if (capture_.isOpened() && capture_.read(frame) && !frame.empty()) {
			drawer_->DrawImg(frame);
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
	//HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here

	// TODO:  Return a different brush if the default is not desired
	//return hbr;

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
	if (bottom_show_) {
		//bottom_tool_->ShowWindow(SW_HIDE);
		dui_bt_->ShowWindow(false, false);
	} else {
		//bottom_tool_->ShowWindow(SW_SHOW);
		dui_bt_->ShowWindow(true, true);
	}

	bottom_show_ = !bottom_show_;

	CDialogEx::OnLButtonDown(nFlags, point);
}

void CvrmfcDlg::handle_com()
{
	//AUTO_LOG_FUNCTION;
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
	CDuiConfirmExitDlg dlg(L"confirm_exit.xml");
	dlg.Create(m_hWnd, L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
	dlg.ShowModal();
	if (dlg.confirmed_) {
		PostMessage(WM_CLOSE);
	}
}

void CvrmfcDlg::do_record()
{
	AUTO_LOG_FUNCTION;
	JLOG_INFO("recording_={}", recording_);
	if (recording_) { return; }

	auto s = now_to_string();
	std::replace(s.begin(), s.end(), ' ', '_');
	std::replace(s.begin(), s.end(), ':', '-');
	auto vfile = get_exe_path_a() + "\\" + s + ".mp4";
}

void CvrmfcDlg::do_capture()
{
	AUTO_LOG_FUNCTION;

}

void CvrmfcDlg::do_file_manager()
{
	AUTO_LOG_FUNCTION;

}

void CvrmfcDlg::do_settings()
{
	AUTO_LOG_FUNCTION;
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

