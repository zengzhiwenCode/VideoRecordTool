#pragma once
#include "duilib.h"


class CDuiVideoPos :
	public CXMLWnd
{
public:
	explicit CDuiVideoPos(const wchar_t* xml);
	virtual ~CDuiVideoPos();

	virtual void InitWindow() override;
	virtual LPCTSTR GetWindowClassName() const override { return L"CDuiVideoPos"; }
	virtual void Notify(DuiLib::TNotifyUI& msg) override;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnClick(TNotifyUI& msg) override;

	void SetPos(int pos);

protected:
	CSliderUI* slider_ = {};


	DUI_DECLARE_MESSAGE_MAP();
};

