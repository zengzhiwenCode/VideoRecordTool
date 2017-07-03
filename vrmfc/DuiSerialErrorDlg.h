#pragma once
#include "duilib.h"
class CDuiSerialErrorDlg :
	public CXMLWnd
{
public:
	std::wstring error_str_ = {};

	static void show_error(HWND hWnd, const std::wstring& err_msg);

	explicit CDuiSerialErrorDlg(const wchar_t* xml);
	virtual ~CDuiSerialErrorDlg();

	virtual void InitWindow() override;
	virtual LPCTSTR GetWindowClassName() const override { return L"CDuiSerialErrorDlg"; }
	virtual void Notify(DuiLib::TNotifyUI& msg) override;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnClick(TNotifyUI& msg) override;
















	DUI_DECLARE_MESSAGE_MAP();
};

