#pragma once
#include "duilib.h"

class CDuiPicDetailDlg :
	public CXMLWnd
{
public:

	struct picinfo {
		std::wstring name = {};
		std::wstring filesize = {};
		std::wstring resolution = {};
		std::wstring ext = {};
		std::wstring create_time = {};
	};

	picinfo picinfo_ = {};

	explicit CDuiPicDetailDlg(const wchar_t* xml);
	virtual ~CDuiPicDetailDlg();

	virtual void InitWindow() override;
	virtual LPCTSTR GetWindowClassName() const override { return L"CDuiPicDetailDlg"; }
	virtual void Notify(DuiLib::TNotifyUI& msg) override;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnClick(TNotifyUI& msg) override;




	DUI_DECLARE_MESSAGE_MAP();
};

