#pragma once

#include "duilib.h"

class CDuiPreviewCaptureDlg;
class CDuiFileManagerDlg;
class CAlarmTextDlg;
class CDuiVideoPlayer;
class CDuiVideoPos;

class CDuiBottomTool : public CXMLWnd
{
public:
	explicit CDuiBottomTool(const wchar_t* xmlpath);
	virtual ~CDuiBottomTool();

	//DUI_DECLARE_MESSAGE_MAP();
	virtual void InitWindow() override;
	virtual LPCTSTR GetWindowClassName() const override { return L"CDuiBottomTool"; }
	virtual void Notify(DuiLib::TNotifyUI& msg) override;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnClick(TNotifyUI& msg) override;

protected:
	fv pics_ = {};
	fv videos_ = {};
	fviters piters_ = {};
	fviters viters_ = {};

	CRect rc_maindlg_ = {};
	CRect rc_filedlg_ = {};
	std::shared_ptr<CDuiFileManagerDlg> file_dlg_ = {};

	SIZE sz_prevpic_ = {};
	std::shared_ptr<CDuiPreviewCaptureDlg> pic_viewer_ = {};
	std::shared_ptr<CAlarmTextDlg> pic_view_tip_ = {};

	std::shared_ptr<CDuiVideoPlayer> video_player_ = {};
	std::shared_ptr<CAlarmTextDlg> video_view_tip_ = {};
	std::shared_ptr<CAlarmTextDlg> video_cur_time_ = {};
	std::shared_ptr<CAlarmTextDlg> video_total_time_ = {};
	std::shared_ptr<CDuiVideoPos> video_slider_ = {};
	CRect rc_video_slider_up_ = {};
	CRect rc_video_slider_down_ = {};


	bool sel_all_ = false;

public:
	enum mode {
		mainwnd,
		filemgr,
		pic_view,
		video_view,
	}mode_ = mainwnd;

	void set_rc_maindlg(const CRect& rc) { rc_maindlg_ = rc; }
	void set_mode(mode m);
	mode get_mode() const { return mode_; }
	void enable_btns(bool able);
	void update_pic_sel(fv pics, fviters iters);
	void update_video_sel(fv videos, fviters iters);
	bool show_pic_tip(bool show);
	bool show_video_tips(bool show);
	bool on_video_pos_changed(const std::wstring& cur, const std::wstring& total, int pos);
	bool on_user_change_video_pos(int pos);
protected:
	void view_pic();
	void view_video();
	void del_pic();
	void del_video();
	void pic_view_dec();
	void pic_view_pic_detail();
	void video_view_del();
	void copy_to_usb();
	void copy_to_usb(char root);
	void file_back_to_main();
	void update_file_mode_btns();
};
