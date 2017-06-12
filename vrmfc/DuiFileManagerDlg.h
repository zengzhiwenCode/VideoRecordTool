#pragma once
#include "duilib.h"

class CDuiFileManagerDlg :
	public CXMLWnd
{
public:
	explicit CDuiFileManagerDlg(const wchar_t* xmlpath);
	virtual ~CDuiFileManagerDlg();
	
	virtual void InitWindow() override;
	virtual LPCTSTR GetWindowClassName() const override { return L"CDuiFileManagerDlg"; }
	virtual void Notify(DuiLib::TNotifyUI& msg) override;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnClick(TNotifyUI& msg) override;

	DUI_DECLARE_MESSAGE_MAP();
};

