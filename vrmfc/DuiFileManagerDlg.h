#pragma once
#include "duilib.h"

class CDuiFileManagerDlg :
	public CXMLWnd
{
public:
	explicit CDuiFileManagerDlg(const wchar_t* xmlpath);
	virtual ~CDuiFileManagerDlg();
	
	virtual void InitWindow() override;
	virtual LPCTSTR GetWindowClassName() const override { return L"CDuiFileManagerDlg"; }
	virtual void Notify(DuiLib::TNotifyUI& msg) override;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnClick(TNotifyUI& msg) override;

	DUI_DECLARE_MESSAGE_MAP();

protected:
	enum filter {
		all,
		pic,
		video
	};

	static filter filter_;

	fv pics_ = {};
	fv videos_ = {};
	fv all_ = {};

	using pic_control_type = COptionUI;
	using btnmap = std::map<fs::path, std::pair<pic_control_type*, bool>>;
	btnmap picbtns_ = {};
	btnmap videobtns_ = {};
	btnmap allbtns_ = {};

	void update_content(filter f);


public:
	void update_filter();
	void scroll_page(int step);
	void sel_all(bool all = true);
	void del_pic(fviters piters);
	void del_video(fviters viters);
};

