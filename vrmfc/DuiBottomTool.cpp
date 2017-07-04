#include "stdafx.h"
#include "vrmfc.h"
#include "DuiBottomTool.h"
#include "vrmfcDlg.h"
#include "DuiFileManagerDlg.h"
#include "DuiPreviewCaptureDlg.h"
#include "DuiMenu.h"
#include "AlarmTextDlg.h"
#include "DuiPicDetailDlg.h"
#include "DuiVideoPlayer.h"
#include "DuiVideoPos.h"
#include "DuiVideoDetailDlg.h"
#include "DuiUsbProgressDlg.h"


namespace {

class auto_change_btn_bk {
public:
	explicit auto_change_btn_bk(CButtonUI* btn)
		: btn(btn)
	{
		btn->SetBkImage(L"image/btnbk_hot.png");
		btn->Invalidate();
	}

	~auto_change_btn_bk()
	{
		btn->SetBkImage(L"image/btnbk_normal.png");
	}
private:
	CButtonUI* btn;
};

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

CDuiVideoDetailDlg::videoinfo g_video_info = {};

}


DUI_BEGIN_MESSAGE_MAP(CDuiBottomTool, CNotifyPump)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()


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
	__super::Notify(msg);
}

LRESULT CDuiBottomTool::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_TIMER && wParam == 1) {
		KillTimer(m_hWnd, 1);
		CButtonUI* rec = static_cast<CButtonUI*>(m_PaintManager.FindControl(btn_names::rec));
		if (rec) {
			rec->SetEnabled(true);
		}
	}
	return __super::HandleMessage(uMsg, wParam, lParam);
}

void CDuiBottomTool::OnClick(TNotifyUI & msg)
{
	__super::OnClick(msg);
	//std::string name = utf8::w2a(msg.pSender->GetName().GetData()); range_log rl("CDuiBottomTool::OnClick " + name);
	std::wstring name = msg.pSender->GetName().GetData();
	if (name.empty()) { return; }
	auto maindlg = static_cast<CvrmfcDlg*>(AfxGetApp()->GetMainWnd()); assert(maindlg);
	if (!maindlg) { JLOG_CRTC("cannot find main dlg!"); return; }
	auto btn = static_cast<CButtonUI*>(msg.pSender);

	auto hot_btn = [&btn]() {
		btn->SetBkImage(L"image/btnbk_hot.png");
	};

	auto normal_btn = [&btn] {
		btn->SetBkImage(L"image/btnbk_normal.png");
	};

	switch (mode_) {
	case CDuiBottomTool::mainwnd:
	{
		//auto_change_btn_bk();
		if (name == btn_names::exit) {
			hot_btn();
			maindlg->do_exit_windows();
			normal_btn();
		} else if (name == btn_names::rec) {
			if (maindlg->do_record()) {
				hot_btn();
				btn->SetText(trw(IDS_STRING_STOP).c_str());
			} 
		} else if (name == btn_names::cap) {
			hot_btn();
			maindlg->do_capture();
			normal_btn();
		} else if (name == btn_names::file) {
			if (maindlg->do_file_manager(rc_filedlg_)) {
				set_mode(CDuiBottomTool::mode::filemgr);
			}
		} else if (name == btn_names::set) {
			hot_btn();
			maindlg->do_settings();
			normal_btn();
		} else if (name == btn_names::sys) {
			hot_btn();
			maindlg->do_system_info();
			normal_btn();
		} else if (name == btn_names::bright) {
			hot_btn();
			maindlg->do_adjust_brightness();
			DuiSleep(300);
			normal_btn();
		}

		break;
	}

	case CDuiBottomTool::filemgr:
	{
		assert(file_dlg_);
		if (name == btn_names::back) {
			file_back_to_main();
			return;
		} else if (name == btn_names::edit) {
			if ((static_cast<COptionUI*>(msg.pSender))->IsSelected()) {
				file_dlg_->sel_all(false);
				editting_ = false;
			} else {
				editting_ = true;
			}
		} else if (name == btn_names::filter) {
			hot_btn();
			auto filter = file_dlg_->update_filter();
			switch (filter) {
			case CDuiFileManagerDlg::all:
				btn->SetText(trw(IDS_STRING_ALL).c_str());
				break;
			case CDuiFileManagerDlg::pic:
				btn->SetText(trw(IDS_STRING_PICTURE).c_str());
				break;
			case CDuiFileManagerDlg::video:
				btn->SetText(trw(IDS_STRING_VIDEO).c_str());
				break;
			default:
				break;
			}
			sel_all_ = false;
			DuiSleep(500);
			normal_btn();
			btn->SetText(trw(IDS_STRING_FILTER).c_str());
		} else if (name == btn_names::page_up) {
			hot_btn();
			file_dlg_->scroll_page(-1); 
			DuiSleep(300);
			normal_btn();
		} else if (name == btn_names::page_dn) {
			hot_btn();
			file_dlg_->scroll_page(1);
			DuiSleep(300);
			normal_btn();
		} else if (name == btn_names::sel_all) {
			hot_btn();
			sel_all_ = !sel_all_;
			file_dlg_->sel_all(sel_all_);
			DuiSleep(300);
			normal_btn();
		} else if (name == btn_names::del) {
			hot_btn();
			file_dlg_->del_pic(piters_, false);
			file_dlg_->del_video(viters_);
			DuiSleep(300);
			normal_btn();
		} else if (name == btn_names::cp_to_usb) {
			hot_btn();
			::EnableWindow(file_dlg_->GetHWND(), 0);
			copy_to_usb();
			normal_btn();
			::EnableWindow(file_dlg_->GetHWND(), 1);
		}
		
		break;
	}
		
	case CDuiBottomTool::pic_view:
	{
		if (name == btn_names::back) {
			assert(pic_viewer_); assert(pic_view_tip_);
			pic_viewer_->SendMessageW(WM_CLOSE);
			pic_viewer_.reset();
			pic_view_tip_->SendMessage(WM_CLOSE);
			pic_view_tip_.reset();

			set_mode(filemgr);
		} else if (name == btn_names::prev_pic) {
			if (piters_.size() == 1) {
				if (piters_[0] > 0) {
					piters_[0]--;
					view_pic();
				}
			}
		} else if (name == btn_names::next_pic) {
			if (piters_.size() == 1) {
				if (piters_[0] + 1 < pics_.size()) {
					piters_[0]++;
					view_pic();
				}
			}
		} else if (name == btn_names::del) {
			pic_view_dec();
		} else if (name == btn_names::cp_to_usb) {
			hot_btn();
			::EnableWindow(pic_viewer_->GetHWND(), 0);
			copy_to_usb();
			::EnableWindow(pic_viewer_->GetHWND(), 1);
			normal_btn();
		} else if (name == btn_names::detail) {
			hot_btn();
			pic_view_detail();
			normal_btn();
		}
		
		break;
	}

	case CDuiBottomTool::video_view:
	{
		if (name == btn_names::back) {
			assert(video_player_); assert(video_view_tip_); assert(video_cur_time_); assert(video_total_time_);

			video_player_->SendMessageW(WM_CLOSE);
			video_player_.reset();

			video_view_tip_->SendMessage(WM_CLOSE);
			video_view_tip_.reset();

			video_cur_time_->SendMessage(WM_CLOSE);
			video_cur_time_.reset();

			video_total_time_->SendMessage(WM_CLOSE);
			video_total_time_.reset();

			video_slider_->SendMessageW(WM_CLOSE);
			video_slider_.reset();

			set_mode(filemgr);
		} else if (name == btn_names::prev_video) {
			if (viters_.size() == 1) {
				if (viters_[0] > 0) {
					viters_[0]--;
					view_video();
				}
			}
		} else if (name == btn_names::next_video) {
			if (viters_.size() == 1) {
				if (viters_[0] + 1 < videos_.size()) {
					viters_[0]++;
					view_video();
				}
			}
		} else if (name == btn_names::stop) {
			hot_btn();
			video_player_->stop();
			on_video_pos_changed(L"", L"", -1);
			DuiSleep(300);
			normal_btn();
		} else if (name == btn_names::play) {
			hot_btn();
			if (video_player_->resume() || video_player_->play()) {
				msg.pSender->SetName(btn_names::pause);
				msg.pSender->SetText(trw(IDS_STRING_PAUSE).c_str());
			}
			DuiSleep(300);
			normal_btn();
		} else if (name == btn_names::pause) {
			hot_btn();
			if (video_player_->pause()) {
				msg.pSender->SetName(btn_names::play);
				msg.pSender->SetText(trw(IDS_STRING_PLAY).c_str());
			}
			DuiSleep(300);
			normal_btn();
		} else if (name == btn_names::del) {
			video_view_del();
		} else if (name == btn_names::cp_to_usb) {
			hot_btn();
			::EnableWindow(video_player_->GetHWND(), 0);
			video_player_->stop();
			on_video_pos_changed(L"", L"", -1);
			copy_to_usb();
			::EnableWindow(video_player_->GetHWND(), 1);
			normal_btn();
		} else if (name == btn_names::detail) {
			hot_btn();
			video_view_detail();
			normal_btn();
		}
		
		break;
	}

	default:
		break;
	}
}

void CDuiBottomTool::on_update(const int & lang)
{
	auto update_lang = [this](tv& vv) {
		for (auto v : vv) {
			auto btn = m_PaintManager.FindControl(v.first.c_str());
			if (btn) {
				btn->SetText(v.second.c_str());
			}
		}
	};

	switch (mode_) {
	case CDuiBottomTool::mainwnd:
	{
		tv vv = {
			{ btn_names::exit, trw(IDS_STRING_EXIT) },
			{ btn_names::rec, trw(IDS_STRING_REC) },
			{ btn_names::cap, trw(IDS_STRING_CAP) },
			{ btn_names::file, trw(IDS_STRING_FILE) },
			{ btn_names::set, trw(IDS_STRING_SET) },
			{ btn_names::sys, trw(IDS_STRING_SYSINFO) },
			
		};

		update_lang(vv);

		//vv = { { btn_names::bright, trw(IDS_STRING_BRIGHTNESS) } };
		auto btn = m_PaintManager.FindControl(btn_names::bright);
		if (btn) {
			btn->SetText((trw(IDS_STRING_BRIGHTNESS) + L" " + std::to_wstring(brightness_level_)).c_str());
		}
	}
		break;
	default:
		break;
	}
}

void CDuiBottomTool::set_mode(mode m)
{
	auto container = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(L"container")); assert(container);
	if (!container) { return; }

	container->RemoveAll();
	sel_all_ = false;

	int GAP_WIDHT = 50;
	static const int BTN_ROUND = 60;
	static const SIZE BORDER_RND = { 0, BTN_ROUND };

	auto add_gap = [&container, &GAP_WIDHT]() {
		auto vert = new CVerticalLayoutUI();
		vert->SetFixedWidth(GAP_WIDHT);
		container->Add(vert);
	};

	auto add_btn = [&container](const wchar_t* name, const wchar_t* text, int font_idx = 0) {
		auto btn = new CButtonUI();
		btn->SetName(name);
		btn->SetText(text);
		btn->SetFont(font_idx);
		btn->SetTextColor(0xFFFFFFFF);

		//btn->SetBkColor(0xFF4472C4);
		//btn->SetBorderRound(BORDER_RND);
		//btn->SetSelectedBkColor(0xFFFF9900);
		btn->SetBkImage(L"image/btnbk_normal.png");
		//btn->SetHotImage(L"image/btnbk_hot.png");
		//btn->SetPushedImage(L"image/btnbk_hot.png");
		//btn->SetSelectedImage(L"image/btnbk_hot.png");
		container->Add(btn);
		//return btn;
	};

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
			
		};

		GAP_WIDHT = 10;
		do_create(vv);
		//{ btn_names::bright, trw(IDS_STRING_BRIGHTNESS) }
		add_gap();
		add_btn(btn_names::bright, (trw(IDS_STRING_BRIGHTNESS) + L" " + std::to_wstring(brightness_level_)).c_str());

		editting_ = false;
		sel_all_ = false;
	}
		break;

	case CDuiBottomTool::filemgr:
	{
		tv vv = {
			{ btn_names::back, trw(IDS_STRING_BACK) },
		};

		GAP_WIDHT = 5;
		do_create(vv);

		{
			auto btn = new COptionUI();
			btn->SetName(btn_names::edit);
			btn->SetText(trw(IDS_STRING_EDIT).c_str());
			btn->SetFont(0);
			btn->SetTextColor(0xFFFFFFFF);
			//btn->SetBkColor(0xFF3275EE);
			//btn->SetBorderRound(BORDER_RND);
			btn->SetBkImage(L"image/btnbk_normal.png");
			//btn->SetHotImage(L"image/btnbk_hot.png");
			btn->SetSelectedImage(L"image/btnbk_hot.png");
			container->Add(btn);
		}

		vv = { 
			{ btn_names::filter, trw(IDS_STRING_FILTER) },
			{ btn_names::page_up, trw(IDS_STRING_PAGE_UP) },
			{ btn_names::page_dn, trw(IDS_STRING_PAGE_DN) },
			{ btn_names::sel_all, trw(IDS_STRING_SEL_ALL) },
			{ btn_names::del, trw(IDS_STRING_DELETE) },
		};

		do_create(vv);

		tp p = { btn_names::cp_to_usb, trw(IDS_STRING_CP_TO_USB) };
		add_btn(p.first.c_str(), p.second.c_str(), 1);
		add_gap();

		if (file_dlg_) {
			file_dlg_->SendMessageW(WM_CLOSE);
			file_dlg_.reset();
		}

		if (!file_dlg_) {
			file_dlg_ = std::make_shared<CDuiFileManagerDlg>(L"filemanager.xml");
			file_dlg_->Create(GetHWND(), L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
			::MoveWindow(file_dlg_->GetHWND(), rc_filedlg_.left, rc_filedlg_.top, rc_filedlg_.Width(), rc_filedlg_.Height(), 0);
		}
		file_dlg_->ShowWindow();

		pics_.clear();
		piters_.clear();
		videos_.clear();
		viters_.clear();
		update_file_mode_btns();

		ShowWindow();
	}
		break;

	case CDuiBottomTool::pic_view:
	{
		tv vv = {
			{ btn_names::prev_pic, trw(IDS_STRING_PREV_PIC) },
			{ btn_names::next_pic, trw(IDS_STRING_NEXT_PIC) },
			{ btn_names::detail, trw(IDS_STRING_DETAIL) },
		};

		GAP_WIDHT = 25;
		do_create(vv);

		tp p = { btn_names::cp_to_usb, trw(IDS_STRING_CP_TO_USB) };
		add_btn(p.first.c_str(), p.second.c_str(), 1);

		vv = { 
			{ btn_names::del, trw(IDS_STRING_DELETE) },
			{ btn_names::back, trw(IDS_STRING_BACK) },
		};
		do_create(vv);

		if (file_dlg_) {
			//file_dlg_->SendMessageW(WM_CLOSE);
			//file_dlg_.reset();
			file_dlg_->ShowWindow(false, false);
		}
	}
		break;

	case CDuiBottomTool::video_view:
	{
		tv vv = {
			{ btn_names::prev_video, trw(IDS_STRING_PREV_VIDEO) },
			{ btn_names::next_video, trw(IDS_STRING_NEXT_VIDEO) },
			{ btn_names::pause, trw(IDS_STRING_PAUSE) },
			{ btn_names::stop, trw(IDS_STRING_STOP) },
			{ btn_names::detail, trw(IDS_STRING_DETAIL) },
		};

		GAP_WIDHT = 5;
		do_create(vv);

		tp p = { btn_names::cp_to_usb, trw(IDS_STRING_CP_TO_USB) };
		add_btn(p.first.c_str(), p.second.c_str(), 1);

		vv = {
			{ btn_names::del, trw(IDS_STRING_DELETE) },
			{ btn_names::back, trw(IDS_STRING_BACK) },
		};
		do_create(vv);

		if (file_dlg_) {
			//file_dlg_->SendMessageW(WM_CLOSE);
			//file_dlg_.reset();
			file_dlg_->ShowWindow(false, false);
		}
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

void CDuiBottomTool::update_pic_sel(fv pics, fviters iters)
{
	JLOG_INFO("CDuiBottomTool::update_pic_sel fviters.size={}", iters.size());
	pics_ = pics;
	piters_ = iters;

	update_file_mode_btns();

	if (sel_all_) {
		return;
	}

	if (editting_) {

	} else {
		view_pic();
	}
	
}

void CDuiBottomTool::update_video_sel(fv videos, fviters iters)
{
	JLOG_INFO("CDuiBottomTool::update_video_sel fviters.size={}", iters.size());
	videos_ = videos;
	viters_ = iters;

	update_file_mode_btns();

	if (sel_all_) {
		return;
	}

	if (editting_) {

	} else {
		view_video();
	}
	
}

bool CDuiBottomTool::show_pic_tip(bool show)
{
	if (mode_ == pic_view) {
		ShowWindow(show, show);
		if (pic_view_tip_) {
			show ? pic_view_tip_->Show() : pic_view_tip_->Hide();
		}
		m_PaintManager.NeedUpdate();
		return true;
	}
	return false;
}

bool CDuiBottomTool::show_video_tips(bool show)
{
	AUTO_LOG_FUNCTION;
	if (mode_ == video_view) {
		ShowWindow(show, show);
		
		if (video_view_tip_) {
			show ? video_view_tip_->Show() : video_view_tip_->Hide();
		}

		if (video_cur_time_) {
			show ? video_cur_time_->Show() : video_cur_time_->Hide();
		}

		if (video_total_time_) {
			show ? video_total_time_->Show() : video_total_time_->Hide();
		}

		if (video_slider_) {
			CRect rc = show ? rc_video_slider_up_ : rc_video_slider_down_;
			::SetWindowPos(video_slider_->GetHWND(),
						   HWND_TOPMOST,
						   rc.left,
						   rc.top,
						   rc.Width(),
						   rc.Height(),
						   SWP_SHOWWINDOW);
		}

		m_PaintManager.NeedUpdate();

		return true;
	}
	return false;
}

bool CDuiBottomTool::on_video_pos_changed(const std::wstring & cur, const std::wstring & total, int pos)
{
	AUTO_LOG_FUNCTION;
	if (mode_ == video_view) {
		if (pos == -1) {
			auto pause = m_PaintManager.FindControl(btn_names::pause);
			if (pause) {
				pause->SetName(btn_names::play);
				pause->SetText(trw(IDS_STRING_PLAY).c_str());
			}

			if (video_cur_time_) {
				video_cur_time_->SetText(L"00:00:00");
				video_cur_time_->Invalidate();
			}

			if (video_slider_) {
				video_slider_->SetPos(0);
			}
		} else {
			if (video_cur_time_) {
				video_cur_time_->SetText(cur.c_str());
				video_cur_time_->Invalidate();
			}

			if (video_total_time_) {
				video_total_time_->SetText(total.c_str());
				video_total_time_->Invalidate();
			}

			g_video_info.video_length = total;

			if (video_slider_) {
				video_slider_->SetPos(pos);
			}
		}

		return true;
	}
	return false;
}

bool CDuiBottomTool::on_user_change_video_pos(int pos)
{
	if (mode_ == video_view) {
		if (video_player_) {
			video_player_->set_pos(pos);
		}
		return true;
	}
	return false;
}

void CDuiBottomTool::on_record_stopped()
{
	auto btn = static_cast<CButtonUI*>(m_PaintManager.FindControl(btn_names::rec)); 
	assert(btn);
	if (!btn)return;
	btn->SetBkImage(L"image/btnbk_normal.png");
	btn->SetText(trw(IDS_STRING_REC).c_str());
	btn->SetEnabled(false);
	SetTimer(m_hWnd, 1, 3000, nullptr);
}

void CDuiBottomTool::set_brightness_level(int level)
{
	brightness_level_ = level;
	if (mode_ == mainwnd) {
		get_ctrl(CButtonUI, bright);
		if (bright) {
			bright->SetText((trw(IDS_STRING_BRIGHTNESS) + L" " + std::to_wstring(brightness_level_)).c_str());
		}
	}
}

void CDuiBottomTool::view_pic()
{
	if (piters_.size() != 1) { return; }
	auto path = pics_[piters_[0]];
	cv::Mat mat = cv::imread(path.string());
	if (!mat.empty()) {
		if (sz_prevpic_.cx != mat.cols || sz_prevpic_.cy != mat.rows) {
			if (pic_viewer_) {
				pic_viewer_->SendMessageW(WM_CLOSE);
				pic_viewer_.reset();
			}
		}

		if (!pic_viewer_ && CDuiPreviewCaptureDlg::make_xml(mat.cols, mat.rows)) {
			pic_viewer_ = std::make_shared<CDuiPreviewCaptureDlg>(L"capture.xml");
			pic_viewer_->set_auto_close(false);
			pic_viewer_->Create(AfxGetMainWnd()->GetSafeHwnd(), L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);

			int w = mat.cols;
			int h = mat.rows;

			CRect rcpic;
			rcpic = rc_maindlg_;
			double r = rc_maindlg_.Width() * 1.0 / rc_maindlg_.Height();
			double rr = w * 1.0 / h;

			if (r <= rr) {
				int hh = h * rc_maindlg_.Width() / w;
				int gap = (rc_maindlg_.Height() - hh) / 2;
				rcpic.top += gap;
				rcpic.bottom -= gap;
			} else {
				int ww = w * rc_maindlg_.Height() / h;
				int gap = (rc_maindlg_.Width() - ww) / 2;
				rcpic.left += gap;
				rcpic.right -= gap;
			}

			pic_viewer_->ResizeClient(rcpic.Width(), rcpic.Height());
			pic_viewer_->CenterWindow();

			//ScreenToClient(rcplayer);
			//m_player.MoveWindow(rcplayer);
		}
			
		if (pic_viewer_) {
			pic_viewer_->set_image(path.string());
			pic_viewer_->ShowWindow();
		}

		if (!pic_view_tip_) {
			pic_view_tip_ = std::make_shared<CAlarmTextDlg>();
			pic_view_tip_->Create(IDD_DIALOG_ALARM_TEXT, CWnd::FromHandle(m_hWnd));
			CRect rc(rc_filedlg_);
			rc.left = rc.right - 200;
			rc.bottom = rc.top + 22;
			pic_view_tip_->SetWindowPos(CWnd::FromHandle(HWND_TOPMOST), rc.left, rc.top, rc.Width(), rc.Height(), SWP_SHOWWINDOW);
		}

		if (pic_view_tip_) {
			pic_view_tip_->SetText(path.filename().wstring().c_str());
			pic_view_tip_->Show();
		}

		sz_prevpic_.cx = mat.cols;
		sz_prevpic_.cy = mat.rows;

		set_mode(pic_view);

		show_pic_tip(false);
	}
}

void CDuiBottomTool::view_video()
{
	if (viters_.size() != 1) { return; }
	auto path = videos_[viters_[0]];

	// get video info by opencv
	{
		cv::VideoCapture cap(utf8::u16_to_mbcs(path.wstring()));
		if (cap.isOpened()) {
			g_video_info.name = utf8::mbcs_to_u16(path.filename().string());
			g_video_info.filesize = utf8::a2w(config::format_space(fs::file_size(path)));
			g_video_info.resolution = std::to_wstring(static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH))) 
				+ L"*" + std::to_wstring(static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT)));
			g_video_info.ext = utf8::a2w(VR_VIDEO_EXT).substr(1);
			g_video_info.fps = std::to_wstring(static_cast<int>(cap.get(cv::CAP_PROP_FPS)));
			g_video_info.create_time = [](const std::wstring& file) {
				WIN32_FILE_ATTRIBUTE_DATA wfad = {};
				SYSTEMTIME st = {};
				GetFileAttributesEx(file.c_str(), GetFileExInfoStandard, &wfad);
				FileTimeToSystemTime(&wfad.ftCreationTime, &st);
				COleDateTime dt(st);
				dt += COleDateTimeSpan(0, 8, 0, 0);
				std::wstring s = dt.Format(L"%Y-%m-%d %H:%M:%S").GetBuffer();
				return s;
			}(path.wstring());
		}
	}
	
	if (!video_view_tip_) {
		video_view_tip_ = std::make_shared<CAlarmTextDlg>();
		video_view_tip_->Create(IDD_DIALOG_ALARM_TEXT, CWnd::FromHandle(m_hWnd));
		CRect rc(rc_filedlg_);
		rc.left = rc.right - 200;
		rc.bottom = rc.top + 22;
		video_view_tip_->SetWindowPos(CWnd::FromHandle(HWND_TOPMOST), rc.left, rc.top, rc.Width(), rc.Height(), SWP_SHOWWINDOW);
	}

	if (video_view_tip_) {
		video_view_tip_->SetText(path.filename().wstring().c_str());
		video_view_tip_->Show();
	}

	if (!video_cur_time_) {
		video_cur_time_ = std::make_shared<CAlarmTextDlg>();
		video_cur_time_->Create(IDD_DIALOG_ALARM_TEXT, CWnd::FromHandle(m_hWnd));
		CRect rc(rc_maindlg_);
		rc.bottom -= 20;
		rc.top = rc.bottom - 22;
		rc.left += 15;
		rc.right = rc.left + 100;
		video_cur_time_->SetWindowPos(CWnd::FromHandle(HWND_TOPMOST), rc.left, rc.top, rc.Width(), rc.Height(), SWP_SHOWWINDOW);
		video_cur_time_->SetBkColor(RGB(91,155,213));
	}

	if (video_cur_time_) {
		video_cur_time_->Show();
	}

	if (!video_total_time_) {
		video_total_time_ = std::make_shared<CAlarmTextDlg>();
		video_total_time_->Create(IDD_DIALOG_ALARM_TEXT, CWnd::FromHandle(m_hWnd));
		CRect rc(rc_maindlg_);
		rc.bottom -= 25;
		rc.top = rc.bottom - 22;
		rc.right -= 15;
		rc.left = rc.right - 100;
		video_total_time_->SetWindowPos(CWnd::FromHandle(HWND_TOPMOST), rc.left, rc.top, rc.Width(), rc.Height(), SWP_SHOWWINDOW);
		video_total_time_->SetBkColor(RGB(91, 155, 213));
	}

	if (video_total_time_) {
		video_total_time_->Show();
	}

	if (!video_slider_) {
		CRect rc(rc_maindlg_);
		rc.bottom -= 5;
		rc.top = rc.bottom - 20;
		rc.left += 5;
		rc.right -= 5;
		
		rc_video_slider_down_ = rc;

		CRect rc_bt;
		GetWindowRect(m_hWnd, rc_bt);
		rc.bottom = rc_bt.top - 5;
		rc.top = rc.bottom - 20;
		rc_video_slider_up_ = rc;

		video_slider_ = std::make_shared<CDuiVideoPos>(L"videoslider.xml");
		video_slider_->Create(AfxGetMainWnd()->GetSafeHwnd(), L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
		::SetWindowPos(video_slider_->GetHWND(), 
					   HWND_TOPMOST, 
					   rc_video_slider_down_.left, 
					   rc_video_slider_down_.top, 
					   rc_video_slider_down_.Width(), 
					   rc_video_slider_down_.Height(), 
					   SWP_SHOWWINDOW);
	}

	if (video_slider_) {
		video_slider_->ShowWindow();
	}

	if (!video_player_) {
		video_player_ = std::make_shared<CDuiVideoPlayer>(L"videoplayer.xml");
		video_player_->Create(AfxGetMainWnd()->GetSafeHwnd(), L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
	}

	video_player_->ShowWindow();
	video_player_->play(utf8::w2a(path.wstring()));

	set_mode(video_view);

	show_video_tips(false);
}

void CDuiBottomTool::del_pic()
{
	if (piters_.empty()) { return; }
	file_dlg_->del_pic(piters_);
}

void CDuiBottomTool::del_video()
{
	if (viters_.empty()) { return; }
	file_dlg_->del_video(viters_);
}

void CDuiBottomTool::pic_view_dec()
{
	if (piters_.size() != 1) { return; }
	auto p = pics_[piters_[0]];
	std::error_code ec;
	fs::remove(p, ec);
	if (ec) {
		JLOG_ERRO(ec.message());
		return;
	}

	if (pic_viewer_) {
		pic_viewer_->ShowWindow(false, false);
	}

	if (pic_view_tip_) {
		pic_view_tip_->Hide();
	}

	set_mode(filemgr);
}

void CDuiBottomTool::pic_view_detail()
{
	if (piters_.size() != 1) { return; }
	auto path = pics_[piters_[0]];
	CDuiPicDetailDlg picdetail(L"picdetail.xml");
	picdetail.picinfo_.name = utf8::mbcs_to_u16(path.filename().string());
	picdetail.picinfo_.filesize = utf8::a2w(config::format_space(fs::file_size(path)));
	cv::Mat mat = cv::imread(utf8::u16_to_mbcs(path.wstring()));
	if (mat.data) {
		picdetail.picinfo_.resolution = std::to_wstring(mat.cols) + L"*" + std::to_wstring(mat.rows);
	}
	picdetail.picinfo_.ext = utf8::a2w(VR_CAPTRUE_EXT).substr(1);
	picdetail.picinfo_.create_time = [](const std::wstring& file) {
		WIN32_FILE_ATTRIBUTE_DATA wfad = {};
		SYSTEMTIME st = {};
		GetFileAttributesEx(file.c_str(), GetFileExInfoStandard, &wfad);
		FileTimeToSystemTime(&wfad.ftCreationTime, &st);
		COleDateTime dt(st);
		dt += COleDateTimeSpan(0, 8, 0, 0);
		std::wstring s = dt.Format(L"%Y-%m-%d %H:%M:%S").GetBuffer();
		return s;
	}(path.wstring());
	picdetail.Create(pic_viewer_->GetHWND(), L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
	::EnableWindow(pic_viewer_->GetHWND(), false);
	::EnableWindow(GetHWND(), false);
	picdetail.ShowModal();
	::EnableWindow(pic_viewer_->GetHWND(), true);
	::EnableWindow(GetHWND(), true);
}

void CDuiBottomTool::video_view_del()
{
	if (video_player_) {
		video_player_->stop();
		video_player_->ShowWindow(false, false);
	}

	show_video_tips(false);

	if (video_slider_) {
		video_slider_->ShowWindow(false, false);
	}

	if (viters_.size() != 1) { return; }
	auto p = videos_[viters_[0]];
	std::error_code ec;
	fs::remove(p, ec);
	if (ec) {
		JLOG_ERRO(ec.message());
		return;
	}

	set_mode(filemgr);
}

void CDuiBottomTool::video_view_detail()
{
	if (viters_.size() != 1) { return; }
	auto path = videos_[viters_[0]];
	CDuiVideoDetailDlg videodetail(L"videodetail.xml");
	videodetail.videoinfo_ = g_video_info;
	videodetail.Create(video_player_->GetHWND(), L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
	::EnableWindow(video_player_->GetHWND(), false);
	::EnableWindow(GetHWND(), false);
	videodetail.ShowModal();
	::EnableWindow(video_player_->GetHWND(), true);
	::EnableWindow(GetHWND(), true);
}

HRESULT CopyFiles(HWND hwnd, const std::vector<std::pair<std::wstring, std::wstring>>& param)
{
	//
	// Initialize COM as STA.
	//
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr)) {
		IFileOperation *pfo;

		//
		// Create the IFileOperation interface 
		//
		hr = CoCreateInstance(CLSID_FileOperation,
							  NULL,
							  CLSCTX_ALL,
							  IID_PPV_ARGS(&pfo));
		if (SUCCEEDED(hr)) {
			//
			// Set the operation flags. Turn off all UI from being shown to the
			// user during the operation. This includes error, confirmation,
			// and progress dialogs.
			//
			hr = pfo->SetOperationFlags(FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_SIMPLEPROGRESS);
			if (SUCCEEDED(hr)) {

				hr = pfo->SetOwnerWindow(hwnd);

				for (auto p : param) {
					//
					// Create an IShellItem from the supplied source path.
					//
					IShellItem *psiFrom = NULL;
					hr = SHCreateItemFromParsingName(p.first.c_str(),
													 NULL,
													 IID_PPV_ARGS(&psiFrom));
					if (SUCCEEDED(hr)) {
						IShellItem *psiTo = NULL;

						if (NULL != p.second.c_str()) {
							//
							// Create an IShellItem from the supplied 
							// destination path.
							//
							hr = SHCreateItemFromParsingName(p.second.c_str(),
															 NULL,
															 IID_PPV_ARGS(&psiTo));
						}

						if (SUCCEEDED(hr)) {
							//
							// Add the operation
							//
							hr = pfo->CopyItem(psiFrom, psiTo, nullptr, NULL);

							if (NULL != psiTo) {
								psiTo->Release();
							}
						}

						psiFrom->Release();
					}
				}

				if (SUCCEEDED(hr)) {
					//
					// Perform the operation to copy the file.
					//
					hr = pfo->PerformOperations();
				}
			}

			//
			// Release the IFileOperation interface.
			//
			pfo->Release();
		}

		CoUninitialize();
	}
	return hr;
}

void CDuiBottomTool::copy_to_usb()
{
	auto ul = config::list_removable_drives();
	if (ul.empty()) { return; }
	if (ul.size() > 1) {
		std::list<std::pair<std::wstring, std::wstring>> items;
		for (auto root : ul) {
			std::wstring name = L"menu_";
			name.push_back(root.first);
			std::wstring text;
			text.push_back(root.first);
			text += L": ";
			text += utf8::mbcs_to_u16(root.second);
			items.push_back(std::make_pair(name, text));
		}
		
		POINT pt = {};
		GetCursorPos(&pt);

		if (file_dlg_) {
			::EnableWindow(file_dlg_->GetHWND(), false);
		}

		CDuiMenu::make_xml(items);
		CDuiMenu menu(L"menu.xml");
		menu.Create(m_hWnd, L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
		CRect rc;
		::GetWindowRect(menu.GetHWND(), &rc);
		::MoveWindow(menu.GetHWND(), pt.x - rc.Width(), pt.y - rc.Height() - 50, rc.Width(), rc.Height(), TRUE);
		menu.ShowModal();

		if (file_dlg_) {
			::EnableWindow(file_dlg_->GetHWND(), true);
		}

		auto pos = menu.selected_.find(L"menu_");
		if (pos != std::wstring::npos) {
			auto root = menu.selected_.substr(pos + 5);
			if (root.size() == 1) {
				auto r = utf8::w2a(root);
				copy_to_usb(r[0]);
			}
		}

	} else {
		copy_to_usb(ul[0].first);
	}
}

void CDuiBottomTool::copy_to_usb(char root)
{
	std::string sroot;
	sroot.push_back(root);
	sroot += ":/";
	sroot += utf8::u16_to_mbcs(utf8::a2w(VR_ROOT_FOLDER));
	std::error_code ec;
	auto uroot = fs::path(sroot);
	if (!fs::is_directory(uroot)) {
		fs::create_directory(uroot, ec);
		if (ec) {
			JLOG_ERRO(ec.message());
			return;
		}
	}

	std::vector<std::pair<std::wstring, std::wstring>> v;
	if (!piters_.empty()) {
		
		auto proot = uroot / utf8::u16_to_mbcs(utf8::a2w(VR_CAPTURE_FOLDER));
		if (!fs::is_directory(proot)) {
			fs::create_directory(proot, ec);
			if (ec) {
				JLOG_ERRO(ec.message());
				return;
			}
		}

		auto dst_pic_folder = utf8::mbcs_to_u16(proot.string());

		for (auto i : piters_) {
			v.push_back(std::make_pair(utf8::mbcs_to_u16(pics_[i].string()), dst_pic_folder));
		}
		//CopyFiles(m_hWnd, v);
	}

	if (!viters_.empty()) {
		//std::vector<std::pair<std::wstring, std::wstring>> v;
		auto vroot = uroot / utf8::u16_to_mbcs(utf8::a2w(VR_VIDEO_FOLDER));
		if (!fs::is_directory(vroot)) {
			fs::create_directory(vroot, ec);
			if (ec) {
				JLOG_ERRO(ec.message());
				return;
			}
		}

		auto dst_video_folder = utf8::mbcs_to_u16(vroot.string());

		for (auto i : viters_) {
			v.push_back(std::make_pair(utf8::mbcs_to_u16(videos_[i].string()), dst_video_folder));
		}
		
	}

	//CopyFiles(m_hWnd, v);
	
	auto dlg = std::make_shared<CDuiUsbProgressDlg>(L"usbprogress.xml");
	dlg->files_ = v;
	dlg->Create(m_hWnd, L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
	CRect rc = rc_maindlg_;
	CRect rcdlg;
	::GetWindowRect(dlg->GetHWND(), rcdlg);
	int wgap = (rc.Width() - rcdlg.Width()) / 2;
	int hgap = (rc.Height() - rcdlg.Height()) / 2;
	rc.left += wgap;
	rc.right -= wgap;
	rc.top += hgap;
	rc.bottom -= hgap;
	::SetWindowPos(dlg->GetHWND(), HWND_TOP, rc.left, rc.top, rc.Width(), rc.Height(), SWP_SHOWWINDOW);
	dlg->ShowModal();
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
	on_record_stopped();
}

void CDuiBottomTool::update_file_mode_btns()
{
	auto total_seled = piters_.size() + viters_.size();
	JLOG_INFO("total_seled={}", total_seled);
	//m_PaintManager.FindControl(btn_names::edit)->SetEnabled(piters_.size() + viters_.size() == 1);
	m_PaintManager.FindControl(btn_names::del)->SetEnabled(total_seled > 0);
	m_PaintManager.FindControl(btn_names::cp_to_usb)->SetEnabled(total_seled > 0);
}

