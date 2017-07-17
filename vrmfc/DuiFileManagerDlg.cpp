#include "stdafx.h"
#include "DuiFileManagerDlg.h"
#include "config.h"
#include "vrmfcDlg.h"

namespace {

enum content_tag {
	pic = 1,
	video,
};

auto list_files = [](const fs::path& path) {
	fv v;
	fv to_be_deled;
	fs::directory_iterator end;
	for (fs::directory_iterator iter(path); iter != end; iter++) {
		if (fs::is_regular_file(iter->path())) {
			if (fs::file_size(iter->path()) == 0) {
				to_be_deled.push_back(iter->path());
			} else {
				v.insert(v.begin(), iter->path());
			}
		}
	}

	std::error_code ec;
	for (auto p : to_be_deled) {
		fs::remove(p, ec);
		if (ec) {
			JLOG_ERRO(ec.message());
		}
	}

	return v;
};


}

DUI_BEGIN_MESSAGE_MAP(CDuiFileManagerDlg, CNotifyPump)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()

CDuiFileManagerDlg::filter CDuiFileManagerDlg::filter_ = CDuiFileManagerDlg::filter::all;

CDuiFileManagerDlg::CDuiFileManagerDlg(const wchar_t* xmlpath)
	: CXMLWnd(xmlpath)
{
}


CDuiFileManagerDlg::~CDuiFileManagerDlg()
{
}

void CDuiFileManagerDlg::InitWindow()
{
	auto cfg = config::get_instance();

	pics_ = list_files(cfg->get_capture_path());
	videos_ = list_files(cfg->get_video_path());

	std::copy(pics_.begin(), pics_.end(), std::back_inserter(all_));
	std::copy(videos_.begin(), videos_.end(), std::back_inserter(all_));
	std::sort(all_.begin(), all_.end(), [](const fs::path& p1, const fs::path& p2) {
		return p2.filename() < p1.filename();
	});

	update_content(filter_);
}

void CDuiFileManagerDlg::Notify(DuiLib::TNotifyUI & msg)
{

	__super::Notify(msg);
}

LRESULT CDuiFileManagerDlg::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return __super::HandleMessage(uMsg, wParam, lParam);
}

void CDuiFileManagerDlg::OnClick(TNotifyUI & msg)
{
	auto name = utf8::w2a(msg.pSender->GetName().GetData());
	auto tag = msg.pSender->GetTag();
	auto mainwnd = static_cast<CvrmfcDlg*>(AfxGetApp()->GetMainWnd()); assert(mainwnd);
	auto cfg = config::get_instance();

	switch (tag) {
	case content_tag::pic:
	{
		auto path = cfg->get_capture_path() + "\\" + name + VR_CAPTRUE_EXT;
		//fviter index = 0;
		//for (const auto& p : pics_) {
		//	if (p.string() == path) {
		//		//mainwnd->do_view_pic(pics_, index);
		//		break;
		//	}
		//	index++;
		//}

		bool seled = !(static_cast<pic_control_type*>(msg.pSender))->IsSelected();
		JLOG_INFO("seled={}", seled);
		fviters iters;
		fviter iter = 0;
		for (auto p : pics_) {
			if (path == p) {
				picbtns_[p].second = seled;
			} else {
				picbtns_[p].second = picbtns_[p].first->IsSelected();
			}
			
			if (picbtns_[p].second) {
				iters.push_back(iter);
			}
			iter++;
		}

		mainwnd->do_update_pic_sel(pics_, iters);
	}
		break;

	case content_tag::video:
	{
		auto path = cfg->get_video_path() + "\\" + name + VR_VIDEO_EXT;
		//fviter index = 0;
		//for (const auto& v : videos_) {
		//	if (v.string() == path) {
		//		//mainwnd->do_play_video(videos_, index);
		//		break;
		//	}
		//	index++;
		//}
		//videobtns_[path].second = !(static_cast<pic_control_type*>(msg.pSender))->IsSelected();
		fviters iters;
		fviter iter = 0;
		bool seled = !(static_cast<pic_control_type*>(msg.pSender))->IsSelected();
		JLOG_INFO("seled={}", seled);
		for (auto p : videos_) {
			if (path == p) {
				videobtns_[p].second = seled;
			} else {
				videobtns_[p].second = videobtns_[p].first->IsSelected();
			}
			if (videobtns_[p].second) {
				iters.push_back(iter);
			}
			iter++;
		}
		mainwnd->do_update_video_sel(videos_, iters);
	}
		break;

	default:
		break;
	}

	__super::OnClick(msg);
}

void CDuiFileManagerDlg::update_content(filter f)
{
	AUTO_LOG_FUNCTION;
	auto container = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(L"container")); assert(container);
	if (!container)return;
	container->RemoveAll();

	auto cfg = config::get_instance();

	typedef std::function<std::pair<std::pair<std::string, std::string>, content_tag>(const fs::path& file, pic_control_type* opt)> get_picture;

	auto create_content = [&container](fv& vv, const wchar_t* group, get_picture get) {
		constexpr int window_width = 1000;
		constexpr int max_col = 4;
		constexpr int img_width = 220;
		constexpr int img_height = img_width * 2 / 3;
		constexpr int text_height = 10;
		constexpr int img_text_gap_height = 5;
		constexpr int content_height = img_height + img_text_gap_height + text_height;
		constexpr int gap_width = (window_width - img_width * max_col) / (max_col + 1) + 1;
		constexpr int gap_height = 20;
		const SIZE img_round = { 5,5 };

		int col = 0;
		CHorizontalLayoutUI* line = nullptr;

		auto add_gap_for_line = [&line, gap_width]() {
			auto gap = new CVerticalLayoutUI();
			gap->SetFixedWidth(gap_width);
			line->Add(gap);
		};

		auto add_gap_for_row = [&container, gap_height]() {
			auto gap = new CHorizontalLayoutUI();
			gap->SetFixedHeight(gap_height);
			container->Add(gap);
		};

		for (auto file : vv) {
			if (col == 0) { // new line
				add_gap_for_row();
				line = new CHorizontalLayoutUI();
				line->SetFixedHeight(content_height);
				add_gap_for_line();
				container->Add(line);
			}

			// content
			auto content = new CVerticalLayoutUI();
			content->SetFixedHeight(content_height);
			content->SetFixedWidth(img_width);
			auto pic = new pic_control_type();
			pic->SetName(file.stem().generic_wstring().c_str());
			pic->SetFixedHeight(img_height);
			pic->SetFixedWidth(img_width);
			pic->SetBorderRound(img_round);

			auto res = get(file, pic);
			auto bkimg = res.first.first;
			auto selimg = res.first.second;

			pic->SetBkImage(utf8::mbcs_to_u16(bkimg).c_str());
			pic->SetSelectedImage(utf8::mbcs_to_u16(selimg).c_str());
			//pic->SetGroup(group);
			pic->SetTag(res.second);

			auto text = new CLabelUI();
			text->SetAttribute(L"align", L"center");
			text->SetFixedHeight(text_height);
			text->SetBkColor(container->GetBkColor());
			text->SetText(file.stem().generic_wstring().c_str());
			text->SetTextColor(0xFFFFFFFF);
			content->Add(pic);
			auto vgap = new CHorizontalLayoutUI();
			vgap->SetFixedHeight(img_text_gap_height);
			content->Add(vgap);
			content->Add(text);
			line->Add(content);
			add_gap_for_line();
			

			if (++col == max_col) { // line break
									//add_gap_for_line();
				col = 0;
				continue;
			}
		}

		add_gap_for_row();
	};

	allbtns_.clear();
	picbtns_.clear();
	videobtns_.clear();

	switch (f) {
	case CDuiFileManagerDlg::all:
	{
		create_content(all_, L"all", [cfg, this](const fs::path& p, pic_control_type* opt) {
			allbtns_[p] = std::make_pair(opt, false);
			if (p.parent_path().string() == cfg->get_capture_path()) {
				picbtns_[p] = std::make_pair(opt, false);
				auto selimg = cfg->get_selected_pic(p.string());
				return std::make_pair(std::make_pair(p.string(), selimg), content_tag::pic);
			} else if (p.parent_path().string() == cfg->get_video_path()) {
				videobtns_[p] = std::make_pair(opt, false);
				auto bkimg = cfg->get_thumb_of_video(p.string());
				auto selimg = cfg->get_selected_pic(bkimg);
				return std::make_pair(std::make_pair(bkimg, selimg), content_tag::video);
			} else {
				assert(0); return std::make_pair(std::make_pair(std::string(), std::string()), content_tag::pic);
			}
		});
	}
		break;

	case CDuiFileManagerDlg::pic:
	{
		create_content(pics_, L"image", [cfg, this](const fs::path& p, pic_control_type* opt) {
			picbtns_[p] = std::make_pair(opt, false);
			auto selimg = cfg->get_selected_pic(p.string());
			return std::make_pair(std::make_pair(p.string(), selimg), content_tag::pic);
		});
	}
		break;

	case CDuiFileManagerDlg::video:
	{
		create_content(videos_, L"thumb", [cfg, this](const fs::path& p, pic_control_type* opt) {
			videobtns_[p] = std::make_pair(opt, false);
			auto bkimg = cfg->get_thumb_of_video(p.string());
			auto selimg = cfg->get_selected_pic(bkimg);
			return std::make_pair(std::make_pair(bkimg, selimg), content_tag::video);
		});
	}
		break;

	default:
		break;
	}

	auto mainwnd = static_cast<CvrmfcDlg*>(AfxGetApp()->GetMainWnd()); assert(mainwnd);
	mainwnd->do_update_pic_sel(pics_, fviters());
	mainwnd->do_update_video_sel(videos_, fviters());
}

CDuiFileManagerDlg::filter CDuiFileManagerDlg::update_filter()
{
	switch (filter_) {
	case CDuiFileManagerDlg::all:
		filter_ = pic;
		break;
	case CDuiFileManagerDlg::pic:
		filter_ = video;
		break;
	case CDuiFileManagerDlg::video:
		filter_ = all;
		break;
	default:
		assert(0);
		break;
	}

	update_content(filter_);
	return filter_;
}

void CDuiFileManagerDlg::scroll_page(int step)
{
	auto container = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(L"container")); assert(container);
	if (!container) { return; }
	auto pos = container->GetScrollPos();
	auto range = container->GetScrollRange();
	auto height = container->GetFixedHeight();

	if (step < 0) {
		if (pos.cy - height < 0) {
			pos.cy = 0;
		} else {
			pos.cy -= height;
		}
	} else {
		if (pos.cy + height > range.cy) {
			pos.cy = range.cy;
		} else {
			pos.cy += height;
		}
	}

	container->SetScrollPos(pos);
}

void CDuiFileManagerDlg::sel_all(bool all)
{
	auto mainwnd = static_cast<CvrmfcDlg*>(AfxGetApp()->GetMainWnd()); assert(mainwnd);
	switch (filter_) {
	case CDuiFileManagerDlg::all:
	{
		for (auto p : allbtns_) {
			if (p.second.first) {
				p.second.first->Selected(all);
			}

			p.second.second = all;
		}

		fviters piters; fviters viters;
		if (all) {
			for (fviter i = 0; i < pics_.size(); i++) { piters.push_back(i); }
			for (fviter i = 0; i < videos_.size(); i++) { viters.push_back(i); }
		}

		mainwnd->do_update_pic_sel(pics_, piters);
		mainwnd->do_update_video_sel(videos_, viters);
		break;
	}
	case CDuiFileManagerDlg::pic:
	{
		for (auto p : picbtns_) {
			if (p.second.first) {
				p.second.first->Selected(all);
			}

			p.second.second = all;
		}
		fviters piters;
		if (all) {
			for (fviter i = 0; i < pics_.size(); i++) { piters.push_back(i); }
		}
		mainwnd->do_update_pic_sel(pics_, piters);
	}
		break;
	case CDuiFileManagerDlg::video:
	{
		for (auto p : videobtns_) {
			if (p.second.first) {
				p.second.first->Selected(all);
			}

			p.second.second = all;
		}
		fviters viters;
		if (all) {
			for (fviter i = 0; i < videos_.size(); i++) { viters.push_back(i); }
		}
		mainwnd->do_update_video_sel(videos_, viters);
	}
		break;
	default:
		break;
	}
}

void CDuiFileManagerDlg::del_pic(fviters piters, bool b_update_content)
{
	auto cfg = config::get_instance();
	for (auto i : piters) {
		auto p = pics_[i];
		std::error_code ec;
		fs::remove(p, ec);
		if (ec) {
			JLOG_ERRO(ec.message());
		}

		auto sel = cfg->get_selected_pic(p.string());
		fs::remove(sel, ec);
		if (ec) {
			JLOG_ERRO(ec.message());
		}
	}

	pics_ = list_files(config::get_instance()->get_capture_path());
	all_.clear();
	std::copy(pics_.begin(), pics_.end(), std::back_inserter(all_));
	std::copy(videos_.begin(), videos_.end(), std::back_inserter(all_));
	std::sort(all_.begin(), all_.end(), [](const fs::path& p1, const fs::path& p2) {
		return p1.filename() < p2.filename();
	});

	if (b_update_content) {
		update_content(filter_);
	}
}

void CDuiFileManagerDlg::del_video(fviters viters, bool b_update_content)
{
	auto cfg = config::get_instance();
	for (auto i : viters) {
		auto p = videos_[i];
		std::error_code ec;
		fs::remove(p, ec);
		if (ec) {
			JLOG_ERRO(ec.message());
		}

		auto thumb = cfg->create_new_thumb_path(p.stem().string());
		if (fs::is_regular_file(thumb)) {
			fs::remove(thumb, ec);
			if (ec) {
				JLOG_ERRO(ec.message());
			}
		}

		auto sel = cfg->get_selected_pic(thumb);
		fs::remove(sel, ec);
		if (ec) {
			JLOG_ERRO(ec.message());
		}
	}

	videos_ = list_files(config::get_instance()->get_video_path());
	all_.clear();
	std::copy(pics_.begin(), pics_.end(), std::back_inserter(all_));
	std::copy(videos_.begin(), videos_.end(), std::back_inserter(all_));
	std::sort(all_.begin(), all_.end(), [](const fs::path& p1, const fs::path& p2) {
		return p2.filename() < p1.filename();
	});
	
	if (b_update_content) {
		update_content(filter_);
	}
}


