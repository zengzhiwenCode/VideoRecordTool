#pragma once

#include "duilib.h"


class CDuiFileManagerDlg;

class CDuiBottomTool : public CXMLWnd
{
public:
	explicit CDuiBottomTool(const wchar_t* xmlpath);
	virtual ~CDuiBottomTool();

	std::shared_ptr<CDuiFileManagerDlg> file_dlg_ = {};

	//DUI_DECLARE_MESSAGE_MAP();
	virtual void InitWindow() override;
	virtual LPCTSTR GetWindowClassName() const override { return L"CDuiBottomTool"; }
	virtual void Notify(DuiLib::TNotifyUI& msg) override;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnClick(TNotifyUI& msg) override;

protected:
	

public:
	enum mode {
		mainwnd,
		filemgr,
		view_pic,
		view_video,
	}mode_ = mainwnd;

	void set_mode(mode m);
	void enable_btns(bool able);
};
