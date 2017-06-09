#include "stdafx.h"
#include "DuiConfirmExitDlg.h"

DUI_BEGIN_MESSAGE_MAP(CConfirmExitDlg, CNotifyPump)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()

CConfirmExitDlg::CConfirmExitDlg(const wchar_t * xmlpath)
	: CXMLWnd(xmlpath)
{
}

CConfirmExitDlg::~CConfirmExitDlg()
{
}


void CConfirmExitDlg::InitWindow()
{
	CenterWindow();
}

void CConfirmExitDlg::Notify(DuiLib::TNotifyUI & msg)
{
	__super::Notify(msg);
}

LRESULT CConfirmExitDlg::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return __super::HandleMessage(uMsg, wParam, lParam);
}

void CConfirmExitDlg::OnClick(TNotifyUI & msg)
{
	std::string name = utf8::w2a(msg.pSender->GetName().GetData()); range_log rl("CConfirmExitDlg::OnClick " + name);

	if (name == "ok") {
		confirmed_ = true;
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
		PostMessage(WM_CLOSE);
	} else if (name == "cancel") {
		PostMessage(WM_CLOSE);
	} 

	__super::OnClick(msg);
}
