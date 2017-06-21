#pragma once
#include "duilib.h"

class CDuiVideoDetailDlg :
	public CXMLWnd
{
public:

	struct videoinfo {
		std::wstring name = {};
		std::wstring filesize = {};
		std::wstring resolution = {};
		std::wstring ext = {};
		std::wstring video_length = {};
		std::wstring fps = {};
		std::wstring create_time = {};
	};

	videoinfo videoinfo_ = {};

	explicit CDuiVideoDetailDlg(const wchar_t* xml);
	virtual ~CDuiVideoDetailDlg();

	virtual void InitWindow() override;
	virtual LPCTSTR GetWindowClassName() const override { return L"CDuiVideoDetailDlg"; }
	virtual void Notify(DuiLib::TNotifyUI& msg) override;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnClick(TNotifyUI& msg) override;




	DUI_DECLARE_MESSAGE_MAP();
};

