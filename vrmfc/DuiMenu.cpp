#include "stdafx.h"
#include "DuiMenu.h"
#include <fstream>

DUI_BEGIN_MESSAGE_MAP(CDuiMenu, CNotifyPump)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()


CDuiMenu::CDuiMenu(const wchar_t* xml)
	: CXMLWnd(xml)
	, selected_()
{
}


bool CDuiMenu::make_xml(const std::list<std::pair<std::wstring, std::wstring>>& items)
{
	int height = 60 * items.size();
	std::string content = R"(<?xml version="1.0" encoding="utf-8"?>
<Window bktrans="true" size="200,)" + std::to_string(height) + 
R"(">
    <Font id="0" name="微软雅黑" size="32" bold="false"/>
    <Font id="1" name="宋体" size="12"/>
    <VerticalLayout bkimage="file='menu_bk.png' corner='40,8,8,8'" hole="false">
)";

	content += R"(<HorizontalLayout height="5" />)";
	for (const auto& item : items) {
		content += R"(
<Option name=")" 
			+ utf8::w2a(item.first) 
			+ R"(" text=")" 
			+ utf8::w2a(item.second) 
			+ R"(" font="0" height="50" align="left" valign="center" endellipsis="true" bkcolor="FF3275AA#" borderround="5,5" group="usb" />
)";

		content += R"(<HorizontalLayout height="5" />)";
	}

	content += R"(
</VerticalLayout>
</Window>
)";

	std::ofstream out(utf8::u16_to_mbcs(get_exe_path() + L"\\skin\\menu.xml"));
	if (out) {
		out << content;
		out.close();
		return true;
	}

	return false;
}

CDuiMenu::~CDuiMenu()
{
}

void CDuiMenu::InitWindow()
{
}

void CDuiMenu::Notify(DuiLib::TNotifyUI & msg)
{
	if (msg.sType == L"selectchanged") {
		selected_ = msg.pSender->GetName().GetData();
		PostMessage(WM_CLOSE);
	}

	__super::Notify(msg);
}

LRESULT CDuiMenu::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return __super::HandleMessage(uMsg, wParam, lParam);
}

void CDuiMenu::OnClick(TNotifyUI & msg)
{
	__super::OnClick(msg);
}
