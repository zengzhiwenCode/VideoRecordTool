#include "stdafx.h"
#include "DuiSerialErrorDlg.h"
#include "vrmfc.h"


void CDuiSerialErrorDlg::show_error(HWND hWnd, const std::wstring & err_msg)
{
	CDuiSerialErrorDlg dlg(L"serialerror.xml");
	dlg.error_str_ = err_msg;
	dlg.Create(hWnd, L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
	dlg.ShowModal();
}

CDuiSerialErrorDlg::CDuiSerialErrorDlg(const wchar_t* xml)
	: CXMLWnd(xml)
{
}


CDuiSerialErrorDlg::~CDuiSerialErrorDlg()
{
}

void CDuiSerialErrorDlg::InitWindow()
{
	CenterWindow();

	get_ctrl(CLabelUI, title);
	title->SetText(error_str_.c_str());
}

void CDuiSerialErrorDlg::Notify(DuiLib::TNotifyUI & msg)
{
	__super::Notify(msg);
}

LRESULT CDuiSerialErrorDlg::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return __super::HandleMessage(uMsg, wParam, lParam);
}

void CDuiSerialErrorDlg::OnClick(TNotifyUI & msg)
{
	if (msg.pSender->GetName() == L"ok") {
		PostMessage(WM_CLOSE);
	}
	__super::OnClick(msg);
}













DUI_BEGIN_MESSAGE_MAP(CDuiSerialErrorDlg, CNotifyPump)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()
