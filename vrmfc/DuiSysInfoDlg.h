#pragma once
#include "duilib.h"

class CDuiSysInfoDlg :
	public CXMLWnd
{
public:
	explicit CDuiSysInfoDlg(const wchar_t* xmlpath);
	virtual ~CDuiSysInfoDlg();

	
	virtual void InitWindow() override;
	virtual LPCTSTR GetWindowClassName() const override { return L"CDuiSysInfoDlg"; }
	virtual void Notify(DuiLib::TNotifyUI& msg) override;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnClick(TNotifyUI& msg) override;




	DUI_DECLARE_MESSAGE_MAP();
};

