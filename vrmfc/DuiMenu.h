#pragma once
#include "duilib.h"
class CDuiMenu :
	public CXMLWnd
{
public:
	

	static bool make_xml(const std::list<std::pair<std::wstring, std::wstring>>& items);

	explicit CDuiMenu(const wchar_t* xml);
	virtual ~CDuiMenu(); 
	virtual void InitWindow() override;
	virtual LPCTSTR GetWindowClassName() const override { return L"CDuiMenu"; }
	virtual void Notify(DuiLib::TNotifyUI& msg) override;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnClick(TNotifyUI& msg) override;


	std::wstring selected_;

	DUI_DECLARE_MESSAGE_MAP();
};

