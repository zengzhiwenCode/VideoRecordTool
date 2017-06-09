#pragma once
#include "duilib.h"

class CConfirmExitDlg : public CXMLWnd
{
public:
	explicit CConfirmExitDlg(const wchar_t* xmlpath);
	virtual ~CConfirmExitDlg();

	DUI_DECLARE_MESSAGE_MAP();
	virtual void InitWindow() override;
	virtual LPCTSTR GetWindowClassName() const override { return L"CConfirmExitDlg"; }
	virtual void Notify(DuiLib::TNotifyUI& msg) override;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnClick(TNotifyUI& msg) override;

public:
	bool confirmed_ = false;
};
