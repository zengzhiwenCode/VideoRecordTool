#include "stdafx.h"
#include "DuiSysInfoDlg.h"
#include "config.h"
#include "vrmfc.h"

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

	auto cfg = config::get_instance();
	auto version = static_cast<CLabelUI*>(m_PaintManager.FindControl(L"version")); assert(version);
	if (version) {
		version->SetText((trw(IDS_STRING_SYSVER) + utf8::a2w(cfg->get_version())).c_str());
	}

	auto space = static_cast<CLabelUI*>(m_PaintManager.FindControl(L"space")); assert(space);
	if (space) {
		space->SetText((trw(IDS_STRING_AVSPACE) + utf8::a2w(cfg->get_remainder_space())).c_str());
	}

	get_ctrl(CLabelUI, title);
	get_ctrl(CButtonUI, closebtn);
	title->SetText(trw(IDS_STRING_SYSTEM_INFO).c_str());
	closebtn->SetText(trw(IDS_STRING_CLOSE).c_str());
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
