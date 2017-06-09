#include "stdafx.h"
#include "DuiBottomTool.h"

using DuiLib::CHorizontalLayoutUI;
using DuiLib::CButtonUI;

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
	__super::OnClick(msg);
}
