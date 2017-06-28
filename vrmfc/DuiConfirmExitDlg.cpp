#include "stdafx.h"
#include "DuiConfirmExitDlg.h"

DUI_BEGIN_MESSAGE_MAP(CDuiConfirmExitDlg, CNotifyPump)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()

CDuiConfirmExitDlg::CDuiConfirmExitDlg(const wchar_t * xmlpath)
	: CXMLWnd(xmlpath)
{
}

CDuiConfirmExitDlg::~CDuiConfirmExitDlg()
{
}


void CDuiConfirmExitDlg::InitWindow()
{
	CenterWindow();
}

void CDuiConfirmExitDlg::Notify(DuiLib::TNotifyUI & msg)
{
	__super::Notify(msg);
}

LRESULT CDuiConfirmExitDlg::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return __super::HandleMessage(uMsg, wParam, lParam);
}

void CDuiConfirmExitDlg::OnClick(TNotifyUI & msg)
{
	std::string name = utf8::w2a(msg.pSender->GetName().GetData()); range_log rl("CDuiConfirmExitDlg::OnClick " + name);
	auto btn = static_cast<CButtonUI*>(msg.pSender);

	auto hot_btn = [&btn]() {
		btn->SetBkImage(L"image/option_bk_hot.png");
	};

	auto normal_btn = [&btn] {
		btn->SetBkImage(L"image/option_bk_normal.png");
	};

	if (name == "ok") {
		hot_btn();
		confirmed_ = true;
#ifndef _DEBUG
		LUID luid;
		TOKEN_PRIVILEGES privs;
		HANDLE token;

		OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
		LookupPrivilegeValue(NULL, L"SeShutdownPrivilege", &luid);

		privs.PrivilegeCount = 1;
		privs.Privileges[0].Luid = luid;
		privs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		auto ok = AdjustTokenPrivileges(token, FALSE, &privs, 0, NULL, NULL);
		if (!ok) {
			CString txt;
			txt.Format(L"AdjustTokenPrivileges Error%d", GetLastError());
			MessageBox(m_hWnd, txt, nullptr, 0);
		}
		ok = ExitWindowsEx(EWX_POWEROFF, EWX_FORCEIFHUNG);
		if (!ok) {
			CString txt;
			txt.Format(L"ExitWindowsEx Error%d", GetLastError());
			MessageBox(m_hWnd, txt, nullptr, 0);
		}
#endif

		normal_btn();
		PostMessage(WM_CLOSE);
	} else if (name == "cancel") {
		hot_btn();
		DuiSleep(300);
		normal_btn();
		PostMessage(WM_CLOSE);
	} 

	__super::OnClick(msg);
}
