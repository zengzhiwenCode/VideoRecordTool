#include "stdafx.h"
#include "DuiBottomTool.h"
#include "vrmfcDlg.h"



DUI_BEGIN_MESSAGE_MAP(CDuiBottomTool, CNotifyPump)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()


CDuiBottomTool::CDuiBottomTool(const wchar_t * xmlpath)
	: CXMLWnd(xmlpath)
{
}

CDuiBottomTool::~CDuiBottomTool()
{
}

void CDuiBottomTool::InitWindow()
{
	
	auto container = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(L"container")); assert(container);
	if (!container) { return; }
	int w = container->GetWidth();
	int h = container->GetHeight();

	const int BTN_W = 64;

	/*SIZE sz = { 5, 0 };
	auto btn = new CButtonUI();
	btn->SetBkImage(L"image\\exit.png");
	btn->SetFixedXY(sz);
	btn->SetFixedWidth(BTN_W);
	btn->SetFixedHeight(BTN_W);
	container->Add(btn);*/
}

void CDuiBottomTool::Notify(DuiLib::TNotifyUI & msg)
{
	__super::Notify(msg);
}

LRESULT CDuiBottomTool::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	
	return __super::HandleMessage(uMsg, wParam, lParam);
}

void CDuiBottomTool::OnClick(TNotifyUI & msg)
{
	std::string name = utf8::w2a(msg.pSender->GetName().GetData()); range_log rl("CDuiBottomTool::OnClick " + name);
	auto dlg = static_cast<CvrmfcDlg*>(AfxGetApp()->GetMainWnd()); assert(dlg); 
	if (!dlg) { JLOG_CRTC("cannot find main dlg!"); return; }

	if (name == "exit") {
		dlg->exit_windows();
	} else if (name == "rec") {
		dlg->record();
	} else if (name == "cap") {
		dlg->capture();
	} else if (name == "file") {
		dlg->file_manager();
	} else if (name == "set") {
		dlg->settings();
	} else if (name == "system") {
		dlg->system_info();
	} else if (name == "bright") {
		dlg->adjust_brightness();
	}

	__super::OnClick(msg);
}
