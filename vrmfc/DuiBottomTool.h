#pragma once

#include "duilib.h"

using DuiLib::DUI_MSGMAP;
using DuiLib::DUI_MSGMAP_ENTRY;
using DuiLib::DUI_PMSG;
using DuiLib::DuiSig_end;
using DuiLib::DuiSig_vn;


class CDuiBottomTool : public CXMLWnd
{
public:
	explicit CDuiBottomTool(const wchar_t* xmlpath);
	virtual ~CDuiBottomTool();

	DUI_DECLARE_MESSAGE_MAP();
	virtual void InitWindow() override;
	virtual LPCTSTR GetWindowClassName() const override { return L"bottomtool"; }
	virtual void Notify(DuiLib::TNotifyUI& msg) override;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnClick(TNotifyUI& msg) override;

protected:

};
