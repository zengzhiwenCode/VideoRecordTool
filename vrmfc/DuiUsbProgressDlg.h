#pragma once
#include "duilib.h"
class CDuiUsbProgressDlg :
	public CXMLWnd //,	public dp::observer<int>
{
public:
	explicit CDuiUsbProgressDlg(const wchar_t* xml);
	virtual ~CDuiUsbProgressDlg();

	virtual void InitWindow() override;
	virtual LPCTSTR GetWindowClassName() const override { return L"CDuiUsbProgressDlg"; }
	virtual void Notify(DuiLib::TNotifyUI& msg) override;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnClick(TNotifyUI& msg) override;
	//virtual void on_update(const int& lang);

	std::vector<std::pair<std::wstring, std::wstring>> files_ = {};

protected:
	std::thread thread_ = {};
	bool running_ = true;
	void worker();

	struct cp_progress {
		std::wstring filename;
		std::wstring sfilesize;
		uintmax_t filesz;
		int progress;
	};
	using cpr = std::shared_ptr<cp_progress>;

	std::list<cpr> cprs_ = {};
	std::mutex mutex_ = {};

	std::chrono::steady_clock::time_point begin_ = {};
	uintmax_t total_file_size_ = 0;
	uintmax_t copied_file_size_ = 0;

	void add_cpr(const cpr& p) { 
		std::lock_guard<std::mutex> lg(mutex_); 
		cprs_.push_back(p); 
	}

	bool get_cpr(cpr& p) {
		if (mutex_.try_lock()) {
			std::lock_guard<std::mutex> lg(mutex_, std::adopt_lock);
			if (!cprs_.empty()) {
				p = cprs_.back();
				for (auto pp : cprs_) {
					copied_file_size_ += pp->filesz;
				}

				cprs_.clear();
				return true;
			}
		}
		return false;
	}


	DUI_DECLARE_MESSAGE_MAP();
};

