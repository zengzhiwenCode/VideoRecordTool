#pragma once

#include "duilib.h"
#include <strmif.h>

class CDuiSettingsDlg : public CXMLWnd, public dp::observer<int>
{
public:
	explicit CDuiSettingsDlg(const wchar_t* xmlpath);
	virtual ~CDuiSettingsDlg();

	DUI_DECLARE_MESSAGE_MAP();
	virtual void InitWindow() override;
	virtual LPCTSTR GetWindowClassName() const override { return L"CDuiSettingsDlg"; }
	virtual void Notify(DuiLib::TNotifyUI& msg) override;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnClick(TNotifyUI& msg) override;
	virtual void on_update(const int& lang);

private:
	VideoProcAmpProperty pvideo_ = VideoProcAmp_Brightness;
	CameraControlProperty pcamera_ = CameraControl_Exposure;

	enum dt {
		year,
		month,
		day,
		hour, 
		minute,
		second,
	};
	dt dt_ = year;

	COleDateTime time_ = {};
	
	std::vector<COptionUI*> capmode_btns_ = {};
	std::vector<COptionUI*> resolution_btns_ = {};

protected:
	void on_fps();
	void on_rec_time();
	void on_video_slider();
	void on_camera_slider();

	void on_update_time(int step);
	void on_apply_time();

	void update_time(COleDateTime& st);
};
