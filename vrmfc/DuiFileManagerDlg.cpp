#include "stdafx.h"
#include "DuiFileManagerDlg.h"
#include "config.h"
#include <filesystem>

namespace {




}

DUI_BEGIN_MESSAGE_MAP(CDuiFileManagerDlg, CNotifyPump)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()

CDuiFileManagerDlg::filter CDuiFileManagerDlg::filter_ = CDuiFileManagerDlg::filter::pic;

CDuiFileManagerDlg::CDuiFileManagerDlg(const wchar_t* xmlpath)
	: CXMLWnd(xmlpath)
{
}


CDuiFileManagerDlg::~CDuiFileManagerDlg()
{
}

void CDuiFileManagerDlg::InitWindow()
{
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

	__super::OnClick(msg);
}

void CDuiFileManagerDlg::update_content(filter f)
{
	auto container = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(L"container")); assert(container);
	if (!container)return;
	container->RemoveAll();

	namespace fs = std::experimental::filesystem;
	using fv = std::list<fs::path>;
	auto cfg = config::get_instance();

	auto list_files = [](const fs::path& path) {
		fv v;
		fs::directory_iterator end;
		for (fs::directory_iterator iter(path); iter != end; iter++) {
			if (fs::is_regular_file(iter->path())) {
				v.push_back(iter->path());
			}
		}

		return v;
	};

	auto get_all_pic = [&cfg, list_files]() {
		return list_files(fs::canonical(cfg->get_capture_path()));
	};

	auto get_all_video = [&cfg, list_files] {
		return list_files(fs::canonical(cfg->get_video_path()));
	};

	auto get_all_file = [&cfg, get_all_pic, get_all_video] {
		auto v1 = get_all_pic();
		auto v2 = get_all_video();
		v1.insert(v1.end(), v2.begin(), v2.end());
		//std::sort(v1.begin(), v1.end());
		v1.sort([](const fs::path& p1, const fs::path& p2) {
			return p1.filename() < p2.filename();
		});
		return v1;
	};

	constexpr int window_width = 1000;
	constexpr int max_col = 4;
	constexpr int img_width = 220;
	constexpr int img_height = img_width * 2 / 3;
	constexpr int text_height = 10;
	constexpr int content_height = img_height + text_height;
	constexpr int gap_width = (window_width - img_width * max_col) / (max_col + 1) + 1;
	constexpr int gap_height = 20;
	const SIZE img_round = { 5,5 };

	switch (f) {
	case CDuiFileManagerDlg::all:
	{
		
	}
		break;
	case CDuiFileManagerDlg::pic:
	{
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

		auto files = get_all_pic();
		for (auto file : files) {
			if (col == 0) { // new line
				add_gap_for_row();
				line = new CHorizontalLayoutUI();
				line->SetFixedHeight(content_height);
				add_gap_for_line();
				container->Add(line);
			} 
			
			{ // content
				auto content = new CVerticalLayoutUI();
				content->SetFixedHeight(content_height);
				content->SetFixedWidth(img_width);
				auto pic = new COptionUI();
				pic->SetFixedHeight(img_height);
				pic->SetFixedWidth(img_width);
				pic->SetBorderRound(img_round);
				pic->SetBkImage(file.generic_wstring().c_str());
				pic->SetGroup(L"img");
				auto text = new CLabelUI();
				text->SetFixedHeight(text_height);
				text->SetBkColor(container->GetBkColor());
				text->SetText(file.stem().generic_wstring().c_str());
				content->Add(pic);
				content->Add(text);
				line->Add(content);
				add_gap_for_line();
			}

			if (++col == max_col) { // line break
				//add_gap_for_line();
				col = 0;
				continue;
			}
		}

		add_gap_for_row();
	}
		break;
	case CDuiFileManagerDlg::video:
		break;
	default:
		break;
	}
}
