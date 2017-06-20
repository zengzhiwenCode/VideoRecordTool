#pragma once
#include "duilib.h"
class CDuiPreviewCaptureDlg :
	public CXMLWnd
{
public:
	static bool make_xml(int w, int h);
	
	

	explicit CDuiPreviewCaptureDlg(const wchar_t* xmlpath);
	virtual ~CDuiPreviewCaptureDlg();
	DUI_DECLARE_MESSAGE_MAP();
	virtual void InitWindow() override;
	virtual LPCTSTR GetWindowClassName() const override { return L"CDuiPreviewCaptureDlg"; }
	virtual void Notify(DuiLib::TNotifyUI& msg) override;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnClick(TNotifyUI& msg) override;
protected:
	std::string img_ = {};
	bool auto_close_ = false;
	bool show_tip_ = true;

public:
	void set_auto_close(bool b = true) { auto_close_ = b; }
	void set_image(const std::string& img);
};

