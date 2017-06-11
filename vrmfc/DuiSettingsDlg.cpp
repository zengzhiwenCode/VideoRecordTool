#include "stdafx.h"
#include "DuiSettingsDlg.h"
#include "config.h"

DUI_BEGIN_MESSAGE_MAP(CDuiSettingsDlg, CNotifyPump)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()

CDuiSettingsDlg::CDuiSettingsDlg(const wchar_t * xmlpath)
	: CXMLWnd(xmlpath)
{
}

CDuiSettingsDlg::~CDuiSettingsDlg()
{
}

void CDuiSettingsDlg::InitWindow()
{
	CenterWindow();
}

void CDuiSettingsDlg::Notify(DuiLib::TNotifyUI & msg)
{
	std::string type = utf8::w2a(msg.sType.GetData());
	std::string name = utf8::w2a(msg.pSender->GetName().GetData());
	range_log rl("CDuiSettingsDlg::Notify type=" + type + " name=" + name);
	CTabLayoutUI* options = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("options"))); assert(options);
	auto cfg = config::get_instance();
	if (type == "selectchanged") {
		if (name == "resolution") {
			options->SelectItem(0);
		} else if (name == "record") {
			options->SelectItem(1);
		} else if (name == "video") {
			options->SelectItem(2);
		} else if (name == "camera") {
			options->SelectItem(3);
		} else if (name == "language") { // ÓïÑÔÉèÖÃ
			do {
				options->SelectItem(4);
				auto lang = cfg->get_lang();
				auto opt_chinese = static_cast<COptionUI*>(m_PaintManager.FindControl(L"chinese")); assert(opt_chinese);
				if (!opt_chinese) { JLOG_ERRO("error get chinese"); break; }
				auto opt_english = static_cast<COptionUI*>(m_PaintManager.FindControl(L"english")); assert(opt_chinese);
				if (!opt_english) { JLOG_ERRO("error get english"); break; }
				opt_chinese->Selected(lang == "zh_CN");
				opt_english->Selected(lang == "en");
			} while (false);
		} else if (name == "chinese") {
			cfg->set_lang("zh_CN"); SetThreadUILanguage(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED));
		} else if (name == "english") {
			cfg->set_lang("en"); SetThreadUILanguage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
		} else if (name == "time") {
			options->SelectItem(5);
		} else if (name == "recover") {
			options->SelectItem(6);
		}
	}

	__super::Notify(msg);
}

LRESULT CDuiSettingsDlg::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYUP) {
		if (wParam == VK_ESCAPE) {
			PostMessage(WM_CLOSE);
		}
	}
	return __super::HandleMessage(uMsg, wParam, lParam);
}

void CDuiSettingsDlg::OnClick(TNotifyUI & msg)
{
	//auto name = utf8::w2a(msg.pSender->GetName().GetData());
	//range_log rl("CDuiSettingsDlg::OnClick " + name);

	__super::OnClick(msg);
}
