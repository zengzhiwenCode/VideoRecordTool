#pragma once

#include "duilib.h"
#include <strmif.h>

class CDuiSettingsDlg : public CXMLWnd
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


private:
	VideoProcAmpProperty pvideo_ = VideoProcAmp_Brightness;
	CameraControlProperty pcamera_ = CameraControl_Exposure;

protected:
	void on_fps();
	void on_rec_time();
	void on_video_slider();
	void on_camera_slider();

	//typedef std::function<void(void)> on_video_slider;

};
