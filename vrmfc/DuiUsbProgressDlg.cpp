#include "stdafx.h"
#include "DuiUsbProgressDlg.h"
#include "vrmfc.h"
#include "config.h"


CDuiUsbProgressDlg::CDuiUsbProgressDlg(const wchar_t* xml)
	: CXMLWnd(xml)
{
}


CDuiUsbProgressDlg::~CDuiUsbProgressDlg()
{
}

void CDuiUsbProgressDlg::InitWindow()
{
	CenterWindow();

	get_ctrl(CLabelUI, filename);
	get_ctrl(CLabelUI, filesize);
	get_ctrl(CLabelUI, speed);
	get_ctrl(CLabelUI, etr);

	//get_ctrl(CLabelUI, filename_var);
	//get_ctrl(CLabelUI, filesize_var);
	//get_ctrl(CLabelUI, speed_var);
	//get_ctrl(CLabelUI, etr_var);

	filename->SetText(trw(IDS_STRING_FILENAME).c_str());
	filesize->SetText(trw(IDS_STRING_FILESIZE).c_str());
	speed->SetText(trw(IDS_STRING_SPEED).c_str());
	etr->SetText(trw(IDS_STRING_ETR).c_str());

	/*filename_var->SetText(trw(IDS_STRING_FILENAME).c_str());
	filesize_var->SetText(trw(IDS_STRING_FILENAME).c_str());
	speed_var->SetText(trw(IDS_STRING_FILENAME).c_str());
	etr_var->SetText(trw(IDS_STRING_FILENAME).c_str());*/

	for (auto f : files_) {
		total_file_size_ += fs::file_size(f.first);
	}

	get_ctrl(CProgressUI, progress);
	progress->SetMinValue(0);
	progress->SetMaxValue(files_.size());
	progress->SetValue(0);

	get_ctrl(CLabelUI, filename_var);
	get_ctrl(CLabelUI, filesize_var);
	auto p = files_.front();
	filename_var->SetText(fs::path(p.first).filename().wstring().c_str());
	filesize_var->SetText(utf8::a2w(config::format_space(fs::file_size(p.first))).c_str());

	begin_ = std::chrono::steady_clock::now();
	thread_ = std::thread(&CDuiUsbProgressDlg::worker, this);

	SetTimer(m_hWnd, 1, 100, nullptr);
}

void CDuiUsbProgressDlg::Notify(DuiLib::TNotifyUI & msg)
{
	__super::Notify(msg);
}

LRESULT CDuiUsbProgressDlg::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYDOWN) {
		if (wParam == VK_ESCAPE) {
			PostMessage(WM_CLOSE);
		}
	} else if (uMsg == WM_TIMER && wParam == 1) {
		using namespace chrono;

		cpr p;
		if (get_cpr(p)) {
			get_ctrl(CProgressUI, progress);
			get_ctrl(CLabelUI, filename_var);
			get_ctrl(CLabelUI, filesize_var);
			get_ctrl(CLabelUI, speed_var);
			get_ctrl(CLabelUI, etr_var);

			progress->SetValue(p->progress);
			filename_var->SetText(p->filename.c_str());
			filesize_var->SetText(p->sfilesize.c_str());

			auto now = steady_clock::now();
			auto diff = duration_cast<seconds>(now - begin_);
			auto secs = static_cast<int>(diff.count());
			if (secs > 0) {
				auto per_s = static_cast<uintmax_t>(copied_file_size_ / (1.0 * secs));
				auto spd = config::format_space(per_s) + "/s";
				speed_var->SetText(utf8::a2w(spd).c_str());

				auto remained_file_sz = total_file_size_ - copied_file_size_;
				auto remained_secs = remained_file_sz / per_s;
				auto csecs = seconds(remained_secs);
				auto hours = duration_cast<chrono::hours>(csecs).count();
				auto minutes = duration_cast<chrono::minutes>(csecs).count() % 60;
				auto seconds = static_cast<int>(csecs.count() % 60);

				std::wstringstream ss;
				if (hours > 0) {
					ss << hours << L":";
				}

				ss << setw(2) << setfill(L'0') << minutes << L':'
					<< setw(2) << setfill(L'0') << seconds;

				etr_var->SetText(ss.str().c_str());
			}

			// done
			if (p->progress == files_.size()) {
				KillTimer(m_hWnd, 1);
				running_ = false;
				thread_.join();
				DuiSleep(500);
				PostMessage(WM_CLOSE);
			}
		}
	}
	return __super::HandleMessage(uMsg, wParam, lParam);
}

void CDuiUsbProgressDlg::OnClick(TNotifyUI & msg)
{
	__super::OnClick(msg);
}

void CDuiUsbProgressDlg::worker()
{
	size_t progress = 1;
	for (auto f : files_) {
		if (!running_) { break; }

		auto path = fs::path(f.first);
		JLOG_INFO("copying {}, progress {}", path.filename().string(), progress);
		auto p = std::make_shared<cp_progress>();
		{
			p->filename = fs::path(f.first).filename().wstring();
			p->filesz = fs::file_size(f.first);
			p->sfilesize = utf8::a2w(config::format_space(p->filesz));
			p->progress = progress++;
		}

		std::error_code ec;
		fs::copy_file(f.first, f.second / path.filename(), fs::copy_options::overwrite_existing, ec);
		if (ec) {
			JLOG_ERRO(ec.message());
		}

		add_cpr(p);

		/*auto b = CopyFileW(f.first.c_str(), f.second.c_str(), 1);
		if (!b) {
			JLOG_ERRO("failed, error code={}", GetLastError());
		}*/
	}
}

//void CDuiUsbProgressDlg::on_update(const int & lang)
//{
//	
//}



























DUI_BEGIN_MESSAGE_MAP(CDuiUsbProgressDlg, CNotifyPump)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()

