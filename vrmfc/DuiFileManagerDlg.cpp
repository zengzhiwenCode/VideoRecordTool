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

	const int max_col = 4;

	switch (f) {
	case CDuiFileManagerDlg::all:
	{
		
	}
		break;
	case CDuiFileManagerDlg::pic:
	{
		int col = 0;
		CHorizontalLayoutUI* line = nullptr;
		auto files = get_all_file();
		for (auto file : files) {
#if 0
			// rename 
			{
				auto name = file.stem().string();
				auto npos = std::string::npos;
				auto pos = name.find('.');
				if (pos != npos) {
					name = name.substr(0, pos);
					name.erase(std::remove(name.begin(), name.end(), '-'), name.end());
					name.erase(std::remove(name.begin(), name.end(), ':'), name.end());
					std::replace(name.begin(), name.end(), ' ', '-');

					name = (file.parent_path() / (name + file.extension().string())).string();
					std::rename(file.string().c_str(), name.c_str());
	}


}
#endif

			if (col == 0) {
				line = new CHorizontalLayoutUI();
				auto gap = new CVerticalLayoutUI();
				gap->SetFixedWidth(50);
				line->Add(gap);
			} else if (col == 3) { // line break

			}


		}
	}
		break;
	case CDuiFileManagerDlg::video:
		break;
	default:
		break;
	}
}
