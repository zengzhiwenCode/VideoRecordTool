#include "stdafx.h"
#include "vrmfc.h"
#include "DuiBottomTool.h"
#include "vrmfcDlg.h"
#include "DuiFileManagerDlg.h"
#include "DuiPreviewCaptureDlg.h"
#include "DuiMenu.h"
#include "AlarmTextDlg.h"
#include "DuiPicDetailDlg.h"

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
		assert(file_dlg_);
		if (name == btn_names::back) {
			file_back_to_main();
			return;
		} else if (name == btn_names::edit) {
			view_pic();
		} else if (name == btn_names::filter) {
			file_dlg_->update_filter();
		} else if (name == btn_names::page_up) {
			file_dlg_->scroll_page(-1); 
		} else if (name == btn_names::page_dn) {
			file_dlg_->scroll_page(1);
		} else if (name == btn_names::sel_all) {
			sel_all_ = !sel_all_;
			file_dlg_->sel_all(sel_all_);
		} else if (name == btn_names::del) {
			del_pic();
			del_video();
		} else if (name == btn_names::cp_to_usb) {
			copy_to_usb();
		}
		
		break;
	}
		
	case CDuiBottomTool::pic_view:
	{
		if (name == btn_names::back) {
			assert(pic_view_); assert(pic_view_tip_);
			pic_view_->SendMessageW(WM_CLOSE);
			pic_view_.reset();
			pic_view_tip_->SendMessage(WM_CLOSE);
			pic_view_tip_.reset();

			set_mode(filemgr);
		} else if (name == btn_names::prev_pic) {
			if (piters_.size() == 1) {
				if (piters_[0] > 0) {
					piters_[0]--;
					view_pic(/*--piters_[0]*/);
				}
			}
		} else if (name == btn_names::next_pic) {
			/*if (piter_ + 1 < pics_.size()) {
				view_pic(++piter_);
			}*/
			if (piters_.size() == 1) {
				if (piters_[0] + 1 < pics_.size()) {
					piters_[0]++;
					view_pic(/*--piters_[0]*/);
				}
			}
		} else if (name == btn_names::del) {
			pic_view_dec_pic();
		} else if (name == btn_names::cp_to_usb) {
			copy_to_usb();
		} else if (name == btn_names::detail) {
			pic_view_pic_detail();
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
	sel_all_ = false;

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

		pics_.clear();
		piters_.clear();
		videos_.clear();
		viters_.clear();
		update_file_mode_btns();
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

		//file_dlg_->ShowWindow(false, false);
		if (file_dlg_) {
			file_dlg_->SendMessageW(WM_CLOSE);
			file_dlg_.reset();
		}
		//view_pic(/*piter_*/);
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

void CDuiBottomTool::update_pic_sel(fv pics, fviters iters)
{
	pics_ = pics;
	piters_ = iters;

	update_file_mode_btns();
}

void CDuiBottomTool::update_video_sel(fv videos, fviters iters)
{
	videos_ = videos;
	viters_ = iters;

	update_file_mode_btns();
	//set_mode(video_view);

	//play_video(viter_);
}

bool CDuiBottomTool::show_tip(bool show)
{
	if (mode_ == pic_view) {
		ShowWindow(show, show);
		if (pic_view_tip_) {
			show ? pic_view_tip_->Show() : pic_view_tip_->Hide();
		}
		return true;
	}
	return false;
}

void CDuiBottomTool::view_pic()
{
	if (piters_.size() != 1) { return; }
	auto path = pics_[piters_[0]];
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

		show_tip(false);
	}
}

void CDuiBottomTool::play_video()
{
}

void CDuiBottomTool::del_pic()
{
	if (piters_.empty()) { return; }
	file_dlg_->del_pic(piters_);
}

void CDuiBottomTool::pic_view_dec_pic()
{
	if (piters_.size() != 1)return;
	if (piters_.size() != 1) { return; }
	auto p = pics_[piters_[0]];
	std::error_code ec;
	fs::remove(p, ec);
	if (ec) {
		JLOG_ERRO(ec.message());
		return;
	}

	if (pic_view_) {
		pic_view_->ShowWindow(false, false);
	}

	if (pic_view_tip_) {
		pic_view_tip_->Hide();
	}

	set_mode(filemgr);
}

void CDuiBottomTool::pic_view_pic_detail()
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
	picdetail.Create(pic_view_->GetHWND(), L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
	::EnableWindow(pic_view_->GetHWND(), false);
	::EnableWindow(GetHWND(), false);
	picdetail.ShowModal();
	::EnableWindow(pic_view_->GetHWND(), true);
	::EnableWindow(GetHWND(), true);
}

void CDuiBottomTool::del_video()
{
	if (viters_.empty()) { return; }
	file_dlg_->del_video(viters_);
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
		CopyFiles(m_hWnd, v);
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

	CopyFiles(m_hWnd, v);
	
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

void CDuiBottomTool::update_file_mode_btns()
{
	m_PaintManager.FindControl(btn_names::edit)->SetEnabled(piters_.size() + viters_.size() == 1);
	m_PaintManager.FindControl(btn_names::del)->SetEnabled(piters_.size() + viters_.size() > 0);
	m_PaintManager.FindControl(btn_names::cp_to_usb)->SetEnabled(piters_.size() + viters_.size() > 0);
}

