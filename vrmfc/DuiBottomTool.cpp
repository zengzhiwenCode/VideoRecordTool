#include "stdafx.h"
#include "vrmfc.h"
#include "DuiBottomTool.h"
#include "vrmfcDlg.h"
#include "DuiFileManagerDlg.h"


//DUI_BEGIN_MESSAGE_MAP(CDuiBottomTool, CNotifyPump)
//DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
//DUI_END_MESSAGE_MAP()


CDuiBottomTool::CDuiBottomTool(const wchar_t * xmlpath)
	: CXMLWnd(xmlpath)
{
}

CDuiBottomTool::~CDuiBottomTool()
{
}

void CDuiBottomTool::InitWindow()
{
	auto container = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(L"container")); assert(container);
	if (!container) { return; }
	int w = container->GetWidth();
	int h = container->GetHeight();

	const int BTN_W = 64;

	/*SIZE sz = { 5, 0 };
	auto btn = new CButtonUI();
	btn->SetBkImage(L"image\\exit.png");
	btn->SetFixedXY(sz);
	btn->SetFixedWidth(BTN_W);
	btn->SetFixedHeight(BTN_W);
	container->Add(btn);*/
}

void CDuiBottomTool::Notify(DuiLib::TNotifyUI & msg)
{
	if (msg.sType == L"click") {
		OnClick(msg);
		return;
	}
	//__super::Notify(msg);
}

LRESULT CDuiBottomTool::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	
	return __super::HandleMessage(uMsg, wParam, lParam);
}

void CDuiBottomTool::OnClick(TNotifyUI & msg)
{
	std::string name = utf8::w2a(msg.pSender->GetName().GetData()); range_log rl("CDuiBottomTool::OnClick " + name);
	auto maindlg = static_cast<CvrmfcDlg*>(AfxGetApp()->GetMainWnd()); assert(maindlg);
	if (!maindlg) { JLOG_CRTC("cannot find main dlg!"); return; }

	if (mode_ == mainwnd) {
		if (name == "exit") {
			maindlg->do_exit_windows();
		} else if (name == "rec") {
			maindlg->do_record();
		} else if (name == "cap") {
			maindlg->do_capture();
		} else if (name == "file") {
			CRect rc;
			maindlg->do_file_manager(rc);
			set_mode(CDuiBottomTool::mode::filemgr);
			file_dlg_ = std::make_shared<CDuiFileManagerDlg>(L"filemanager.xml");
			file_dlg_->Create(GetHWND(), L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
			::MoveWindow(file_dlg_->GetHWND(), rc.left, rc.top, rc.Width(), rc.Height(), 0);
			file_dlg_->ShowWindow();
		} else if (name == "set") {
			maindlg->do_settings();
		} else if (name == "system") {
			maindlg->do_system_info();
		} else if (name == "bright") {
			maindlg->do_adjust_brightness();
		}
	} else if (mode_ == filemgr) {
		//assert(dlg_);
		if (name == "back") {
			//dlg_->PostMessageW(WM_CLOSE);
			assert(file_dlg_);
			file_dlg_->SendMessageW(WM_CLOSE);
			file_dlg_.reset();
			maindlg->do_file_manager_over();
			set_mode(mainwnd);
			return;
		}
	}

	//__super::OnClick(msg);
}

void CDuiBottomTool::set_mode(mode m)
{
	auto container = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(L"container")); assert(container);
	if (!container) { return; }

	container->RemoveAll();

	int GAP_WIDHT = 50;
	const int BTN_ROUND = 15;
	const SIZE BORDER_RND = { BTN_ROUND, BTN_ROUND };

	auto add_gap = [&container, &GAP_WIDHT]() {
		auto vert = new CVerticalLayoutUI();
		vert->SetFixedWidth(GAP_WIDHT);
		container->Add(vert);
	};

	auto add_btn = [&container, BORDER_RND](const wchar_t* name, const wchar_t* text, int font_idx = 0) {
		auto btn = new CButtonUI();
		btn->SetName(name);
		btn->SetText(text);
		btn->SetFont(font_idx);
		btn->SetBkColor(0xFF3275EE);
		btn->SetBorderRound(BORDER_RND);
		container->Add(btn);
	};

	// type of pair
	using tp = std::pair<std::wstring, std::wstring>;
	// type of vector
	using tv = std::vector<tp>;

	auto do_create = [add_gap, add_btn](const tv& vv) {
		add_gap();
		for (auto v : vv) {
			add_btn(v.first.c_str(), v.second.c_str());
			add_gap();
		}
	};

	switch (m) {
	case CDuiBottomTool::mainwnd:
	{
		tv vv = { 
			{ L"exit", trw(IDS_STRING_EXIT) },
			{ L"rec", trw(IDS_STRING_REC) },
			{ L"cap", trw(IDS_STRING_CAP) },
			{ L"file", trw(IDS_STRING_FILE) },
			{ L"set", trw(IDS_STRING_SET) },
			{ L"sys", trw(IDS_STRING_SYSINFO) },
			{ L"bright", trw(IDS_STRING_BRIGHTNESS) }
		};

		GAP_WIDHT = 10;
		do_create(vv);
	}
		break;

	case CDuiBottomTool::filemgr:
	{
		tv vv = {
			{ L"back", trw(IDS_STRING_BACK) },
			{ L"edit", trw(IDS_STRING_EDIT) },
			{ L"filter", trw(IDS_STRING_FILTER) },
			{ L"page_up", trw(IDS_STRING_PAGE_UP) },
			{ L"page_dn", trw(IDS_STRING_PAGE_DN) },
			{ L"sel_all", trw(IDS_STRING_SEL_ALL) },
			{ L"delete", trw(IDS_STRING_DELETE) },
			
		};

		GAP_WIDHT = 5;
		do_create(vv);

		tp p = { L"cp_to_usb", trw(IDS_STRING_CP_TO_USB) };
		add_gap();
		add_btn(p.first.c_str(), p.second.c_str(), 1);
		add_gap();
	}
		break;

	case CDuiBottomTool::view_pic:
		break;
	case CDuiBottomTool::view_video:
		break;
	default:
		assert(0);
		return;
		break;
	}

	mode_ = m;
	container->NeedUpdate();
}

void CDuiBottomTool::enable_btns(bool able)
{
	if (mode_ != mainwnd) { assert(0); return; }

	//auto bk_color = able ? RGB(0x32, 0x75, 0xEE) : RGB(0xD7, 0x7E, 0x44);
	auto bk_color = able ? 0xFF3275EE : 0xFFD77E44;

	if (0) {
		auto btn_exit = static_cast<CButtonUI*>(m_PaintManager.FindControl(L"exit"));
		if (btn_exit) {

			btn_exit->SetBkColor(bk_color);
			btn_exit->SetEnabled(able);
			auto container = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(L"container")); assert(container);
			if (!container) { return; }
			container->NeedUpdate();
		}
		return;
	}

#define __apply_dui_bottom_tool_btn(name) \
	CButtonUI* btn_##name = static_cast<CButtonUI*>(m_PaintManager.FindControl(utf8::a2w(#name).c_str())); \
	if (btn_##name) { \
		btn_##name->SetEnabled(able); \
		btn_##name->SetBkColor(bk_color); \
	}

	__apply_dui_bottom_tool_btn(exit);
	__apply_dui_bottom_tool_btn(cap);
	__apply_dui_bottom_tool_btn(file);
	__apply_dui_bottom_tool_btn(set);
	__apply_dui_bottom_tool_btn(system);
	__apply_dui_bottom_tool_btn(bright);

	auto container = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(L"container")); assert(container);
	if (!container) { return; }
	container->NeedUpdate();
}

