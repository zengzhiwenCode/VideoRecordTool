#pragma once

#include "duilib.h"

class CDuiPreviewCaptureDlg;
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
	fv pics_ = {};
	fviter piter_ = {};
	fv videos_ = {};
	fviter viter_ = {};

	std::shared_ptr<CDuiPreviewCaptureDlg> pic_view_ = {};

	

public:
	enum mode {
		mainwnd,
		filemgr,
		pic_view,
		video_view,
	}mode_ = mainwnd;

	void set_mode(mode m);
	mode get_mode() const { return mode_; }
	void enable_btns(bool able);
	void view_pic(fv pics, fviter iter);
	void play_video(fv videos, fviter iter);
	
protected:
	void view_pic(fviter index);
	void play_video(fviter index);
};
