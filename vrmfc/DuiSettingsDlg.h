#pragma once

#include "duilib.h"

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

protected:
	void on_fps();
};
