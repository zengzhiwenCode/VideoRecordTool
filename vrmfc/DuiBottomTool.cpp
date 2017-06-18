#include "stdafx.h"
#include "vrmfc.h"
#include "DuiBottomTool.h"
#include "vrmfcDlg.h"
#include "DuiFileManagerDlg.h"
#include "DuiPreviewCaptureDlg.h"


namespace {

namespace btn_names {

auto exit = L"exit";
auto back = L"back";
auto rec = L"rec";
auto cap = L"cap";
auto file = L"file";
auto set = L"set";
auto sys = L"sys";
auto bright = L"bright";
auto edit = L"edit";
auto filter = L"filter";
auto page_up = L"page_up";
auto page_dn = L"page_dn";
auto sel_all = L"sel_all";
auto del = L"delete";
auto cp_to_usb = L"cp_to_usb";
auto prev_pic = L"prev_pic";
auto next_pic = L"next_pic";
auto detail = L"detail";
auto prev_video = L"prev_video";
auto next_video = L"next_video";
auto stop = L"stop";
auto play = L"play";
auto pause = L"pause";


}

}


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
	//std::string name = utf8::w2a(msg.pSender->GetName().GetData()); range_log rl("CDuiBottomTool::OnClick " + name);
	std::wstring name = msg.pSender->GetName().GetData();
	auto maindlg = static_cast<CvrmfcDlg*>(AfxGetApp()->GetMainWnd()); assert(maindlg);
	if (!maindlg) { JLOG_CRTC("cannot find main dlg!"); return; }

	switch (mode_) {
	case CDuiBottomTool::mainwnd:
	{
		if (name == btn_names::back) {
			maindlg->do_exit_windows();
		} else if (name == btn_names::rec) {
			maindlg->do_record();
		} else if (name == btn_names::cap) {
			maindlg->do_capture();
		} else if (name == btn_names::file) {
			if (maindlg->do_file_manager(rc_filedlg_)) {
				set_mode(CDuiBottomTool::mode::filemgr);
			}
		} else if (name == btn_names::set) {
			maindlg->do_settings();
		} else if (name == btn_names::sys) {
			maindlg->do_system_info();
		} else if (name == btn_names::bright) {
			maindlg->do_adjust_brightness();
		}

		break;
	}

	case CDuiBottomTool::filemgr:
	{
		//assert(dlg_);
		if (name == btn_names::back) {
			file_back_to_main();
			return;
		} else if (name == btn_names::filter) {
			assert(file_dlg_);
			file_dlg_->update_filter();
		} else if (name == btn_names::page_up) {
			scroll_page(-1);
		} else if (name == btn_names::page_dn) {
			scroll_page(1);
		}
		
		break;
	}
		
	case CDuiBottomTool::pic_view:
	{
		if (name == btn_names::back) {
			assert(pic_view_);
			pic_view_->SendMessageW(WM_CLOSE);
			pic_view_.reset();

			set_mode(filemgr);
		} else if (name == btn_names::prev_pic) {
			if (piter_ > 0) {
				view_pic(--piter_);
			}
		} else if (name == btn_names::next_pic) {
			if (piter_ + 1 < pics_.size()) {
				view_pic(++piter_);
			}
		} else if (name == btn_names::del) {
			del_pic(piter_);
		} else if (name == btn_names::cp_to_usb) {

		} else if (name == btn_names::detail) {

		}
		
		break;
	}

	case CDuiBottomTool::video_view:
	{
		if (name == btn_names::back) {
			set_mode(filemgr);
		} else if (name == btn_names::prev_video) {

		} else if (name == btn_names::next_video) {
		
		} else if (name == btn_names::stop) {
		
		} else if (name == btn_names::play) {

		} else if (name == btn_names::pause) {

		} else if (name == btn_names::del) {

		} else if (name == btn_names::cp_to_usb) {

		} else if (name == btn_names::detail) {

		}
		
		break;
	}

	default:
		break;
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
			{ btn_names::exit, trw(IDS_STRING_EXIT) },
			{ btn_names::rec, trw(IDS_STRING_REC) },
			{ btn_names::cap, trw(IDS_STRING_CAP) },
			{ btn_names::file, trw(IDS_STRING_FILE) },
			{ btn_names::set, trw(IDS_STRING_SET) },
			{ btn_names::sys, trw(IDS_STRING_SYSINFO) },
			{ btn_names::bright, trw(IDS_STRING_BRIGHTNESS) }
		};

		GAP_WIDHT = 10;
		do_create(vv);
	}
		break;

	case CDuiBottomTool::filemgr:
	{
		tv vv = {
			{ btn_names::back, trw(IDS_STRING_BACK) },
			{ btn_names::edit, trw(IDS_STRING_EDIT) },
			{ btn_names::filter, trw(IDS_STRING_FILTER) },
			{ btn_names::page_up, trw(IDS_STRING_PAGE_UP) },
			{ btn_names::page_dn, trw(IDS_STRING_PAGE_DN) },
			{ btn_names::sel_all, trw(IDS_STRING_SEL_ALL) },
			{ btn_names::del, trw(IDS_STRING_DELETE) },
			
		};

		GAP_WIDHT = 5;
		do_create(vv);

		tp p = { btn_names::cp_to_usb, trw(IDS_STRING_CP_TO_USB) };
		add_btn(p.first.c_str(), p.second.c_str(), 1);
		add_gap();

		if (!file_dlg_) {
			file_dlg_ = std::make_shared<CDuiFileManagerDlg>(L"filemanager.xml");
			file_dlg_->Create(GetHWND(), L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
			::MoveWindow(file_dlg_->GetHWND(), rc_filedlg_.left, rc_filedlg_.top, rc_filedlg_.Width(), rc_filedlg_.Height(), 0);
		}
		file_dlg_->ShowWindow();
	}
		break;

	case CDuiBottomTool::pic_view:
	{
		tv vv = {
			{ btn_names::prev_pic, trw(IDS_STRING_PREV_PIC) },
			{ btn_names::back, trw(IDS_STRING_BACK) },
			{ btn_names::del, trw(IDS_STRING_DELETE) },

		};

		GAP_WIDHT = 25;
		do_create(vv);

		tp p = { btn_names::cp_to_usb, trw(IDS_STRING_CP_TO_USB) };
		add_btn(p.first.c_str(), p.second.c_str(), 1);

		vv = { 
			{ btn_names::detail, trw(IDS_STRING_DETAIL) },
			{ btn_names::next_pic, trw(IDS_STRING_NEXT_PIC) },
		};
		do_create(vv);

		file_dlg_->ShowWindow(false, false);

		view_pic(piter_);
	}
		break;

	case CDuiBottomTool::video_view:
	{
		tv vv = {
			{ btn_names::prev_video, trw(IDS_STRING_PREV_VIDEO) },
			{ btn_names::stop, trw(IDS_STRING_STOP) },
			{ btn_names::pause, trw(IDS_STRING_PAUSE) },
			{ btn_names::back, trw(IDS_STRING_BACK) },
			{ btn_names::del, trw(IDS_STRING_DELETE) },

		};

		GAP_WIDHT = 5;
		do_create(vv);

		tp p = { btn_names::cp_to_usb, trw(IDS_STRING_CP_TO_USB) };
		add_btn(p.first.c_str(), p.second.c_str(), 1);

		vv = {
			{ btn_names::detail, trw(IDS_STRING_DETAIL) },
			{ btn_names::next_video, trw(IDS_STRING_NEXT_VIDEO) },
		};
		do_create(vv);
	}
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
	__apply_dui_bottom_tool_btn(sys);
	__apply_dui_bottom_tool_btn(bright);

	auto container = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(L"container")); assert(container);
	if (!container) { return; }
	container->NeedUpdate();
}

void CDuiBottomTool::view_pic(fv pics, fviter iter)
{
	pics_ = pics;
	piter_ = iter;

	set_mode(pic_view);
}

void CDuiBottomTool::play_video(fv videos, fviter iter)
{
	videos_ = videos;
	viter_ = iter;

	set_mode(video_view);

	play_video(viter_);
}

bool CDuiBottomTool::is_valid_pic_iter(fviter idx)
{
	return (0 <= idx && idx < pics_.size());
}

bool CDuiBottomTool::is_valid_video_iter(fviter idx)
{
	return (0 <= idx && idx < videos_.size());
}

void CDuiBottomTool::view_pic(fviter index)
{
	if (is_valid_pic_iter(index)) {
		auto path = pics_[index];
		cv::Mat mat = cv::imread(path.string());
		if (!mat.empty()) {
			if (sz_prevpic_.cx != mat.cols || sz_prevpic_.cy != mat.rows) {
				if (pic_view_) {
					pic_view_->SendMessageW(WM_CLOSE);
					pic_view_.reset();
				}
			}

			if (!pic_view_ && CDuiPreviewCaptureDlg::make_xml(mat.cols, mat.rows)) {
				pic_view_ = std::make_shared<CDuiPreviewCaptureDlg>(L"capture.xml");
				pic_view_->set_auto_close(false);
				pic_view_->Create(AfxGetMainWnd()->GetSafeHwnd(), L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
			}
			
			if (pic_view_) {
				pic_view_->set_image(path.string());
				pic_view_->ShowWindow();
			}

			sz_prevpic_.cx = mat.cols;
			sz_prevpic_.cy = mat.rows;
		}
	}
}

void CDuiBottomTool::play_video(fviter index)
{
}

void CDuiBottomTool::del_pic(fviter index)
{
	if (is_valid_pic_iter(index)) {
		auto p = pics_[index];
		std::error_code ec;
		fs::remove(p, ec);
		if (ec) {
			JLOG_ERRO(ec.message());
			return;
		}

		if (pic_view_) {
			pic_view_->ShowWindow(false, false);
		}

		pics_.erase(pics_.begin() + index);
		piter_ = 0;

		assert(file_dlg_);
		file_dlg_->SendMessageW(WM_CLOSE);
		file_dlg_.reset();
		set_mode(filemgr);
	}
}

void CDuiBottomTool::file_back_to_main()
{
	auto maindlg = static_cast<CvrmfcDlg*>(AfxGetApp()->GetMainWnd()); assert(maindlg);
	if (!maindlg) { JLOG_CRTC("cannot find main dlg!"); return; }
	assert(file_dlg_);
	file_dlg_->SendMessageW(WM_CLOSE);
	file_dlg_.reset();
	maindlg->do_file_manager_over();
	set_mode(mainwnd);
}

void CDuiBottomTool::scroll_page(int down)
{
	if (file_dlg_) {
		file_dlg_->scroll_page(down);
	}
}

