#include "stdafx.h"
#include "DuiMenuWnd.h"
#include <fstream>

bool CMenuWnd::make_xml(const std::list<std::pair<std::wstring, std::wstring>>& items)
{
	std::string content = R"(
<?xml version="1.0" encoding="utf-8"?>
<Window bktrans="true" size="120,82">
<Font id="0" name="微软雅黑" size="13" bold="false"/>
<Font id="1" name="宋体" size="12"/>
<MultiLanguage id="1" value="打开"/>
<VerticalLayout bkimage="file='menu_bk.png' corner='40,8,8,8'" hole="false">
	<List name="menu" header="hidden" inset="8,8,8,8" itemhotimage="file='menu_hot_bk.png' corner='2,2,2,2'" itemdisabledbkcolor="#FE00F4FF">

)";

	for (const auto& item : items) {
		content += R"(<ListContainerElement name=")" + utf8::w2a(item.first) + R"(" height="22" inset="40,0,0,0">)";
		content += "\r\n";
		content += R"(<Label text=")" + utf8::w2a(item.second) + R"(" mouse="false"/>)";
		content += "\r\n";
		content += "</ListContainerElement>\r\n";
	}

	content += "</List></VerticalLayout></Window>\r\n";

	std::ofstream out(utf8::u16_to_mbcs(get_exe_path() + L"\\skin\\menu.xml"));
	if (out) {
		out << content;
		out.close();
		return true;
	}

	return false;
}
