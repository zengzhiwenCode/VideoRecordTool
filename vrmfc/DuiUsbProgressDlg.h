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
	int total_kb_ = 0;
	uintmax_t total_file_size_ = 0;
	uintmax_t copied_file_size_ = 0;

	void add_cpr(const cpr& p);

	bool get_cpr(cpr& p);


	DUI_DECLARE_MESSAGE_MAP();
};

