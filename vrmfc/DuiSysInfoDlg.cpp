#include "stdafx.h"
#include "DuiSysInfoDlg.h"


DUI_BEGIN_MESSAGE_MAP(CDuiSysInfoDlg, CNotifyPump)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()


CDuiSysInfoDlg::CDuiSysInfoDlg(const wchar_t* xmlpath)
	: CXMLWnd(xmlpath)
{
}


CDuiSysInfoDlg::~CDuiSysInfoDlg()
{
}

void CDuiSysInfoDlg::InitWindow()
{
	CenterWindow();
}

void CDuiSysInfoDlg::Notify(DuiLib::TNotifyUI & msg)
{
	__super::Notify(msg);
}

LRESULT CDuiSysInfoDlg::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return __super::HandleMessage(uMsg, wParam, lParam);
}

void CDuiSysInfoDlg::OnClick(TNotifyUI & msg)
{
	__super::OnClick(msg);
}
