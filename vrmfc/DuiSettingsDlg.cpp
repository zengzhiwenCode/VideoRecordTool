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
	auto cfg = config::get_instance();
	auto dst = std::to_wstring(cfg->get_video_w()) + L"*" + std::to_wstring(cfg->get_video_h());
	auto names = { L"320*240", L"640*360", L"640*480", L"920*720", L"1280*720" };
	for (auto rname : names) {
		auto opt = static_cast<COptionUI*>(m_PaintManager.FindControl(rname));
		if (opt) {
			opt->Selected(dst == rname);
		}
	}

	do {
		auto lang = cfg->get_lang();
		auto opt_chinese = static_cast<COptionUI*>(m_PaintManager.FindControl(L"chinese")); assert(opt_chinese);
		if (!opt_chinese) { JLOG_ERRO("error get chinese"); break; }
		auto opt_english = static_cast<COptionUI*>(m_PaintManager.FindControl(L"english")); assert(opt_chinese);
		if (!opt_english) { JLOG_ERRO("error get english"); break; }
		opt_chinese->Selected(lang == "zh_CN");
		opt_english->Selected(lang == "en");
	} while (false);
}

void CDuiSettingsDlg::Notify(DuiLib::TNotifyUI & msg)
{
	std::string type = utf8::w2a(msg.sType.GetData());
	std::string name = utf8::w2a(msg.pSender->GetName().GetData());
	//range_log rl("CDuiSettingsDlg::Notify type=" + type + " name=" + name);
	CTabLayoutUI* options = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("options"))); assert(options);
	auto cfg = config::get_instance();
	if (type == "selectchanged") {
		if (name == "resolution") {		// 分辨率/格式设置
			options->SelectItem(0);
		} else if (name == "record") {
			options->SelectItem(1);
		} else if (name == "video") {
			options->SelectItem(2);
		} else if (name == "camera") {
			options->SelectItem(3);
		} else if (name == "language") { // 语言设置
			options->SelectItem(4);
		} else if (name == "chinese") {
			cfg->set_lang("zh_CN"); SetThreadUILanguage(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED));
		} else if (name == "english") {
			cfg->set_lang("en"); SetThreadUILanguage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
		} else if (name == "time") {
			options->SelectItem(5);
		} else if (name == "recover") {
			options->SelectItem(6);
		}
	} else if (type == "valuechanged") {
		if (name == "fps") {
			on_fps();
		} else if (name == "rec_time") {
			on_rec_time();
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
	auto name = utf8::w2a(msg.pSender->GetName().GetData());
	//range_log rl("CDuiSettingsDlg::OnClick " + name);

	if (name == "fps_dec") {
		auto fps = static_cast<CSliderUI*>(m_PaintManager.FindControl(L"fps")); assert(fps);
		if (fps) {
			auto val = fps->GetValue();
			if (val > fps->GetMinValue()) {
				fps->SetValue(--val);
				on_fps();
			}
		}
	} else if (name == "fps_inc") {
		auto fps = static_cast<CSliderUI*>(m_PaintManager.FindControl(L"fps")); assert(fps);
		if (fps) {
			auto val = fps->GetValue();
			if (val < fps->GetMaxValue()) {
				fps->SetValue(++val);
				on_fps();
			}
		}
	} else if (name == "rec_time_dec") {
		auto rec_time = static_cast<CSliderUI*>(m_PaintManager.FindControl(L"rec_time")); assert(rec_time);
		if (rec_time) {
			auto val = rec_time->GetValue();
			if (val > rec_time->GetMinValue()) {
				rec_time->SetValue(--val);
				on_rec_time();
			}
		}
	} else if (name == "rec_time_inc") {
		auto rec_time = static_cast<CSliderUI*>(m_PaintManager.FindControl(L"rec_time")); assert(rec_time);
		if (rec_time) {
			auto val = rec_time->GetValue();
			if (val < rec_time->GetMaxValue()) {
				rec_time->SetValue(++val);
				on_rec_time();
			}
		}
	}


	__super::OnClick(msg);
}

void CDuiSettingsDlg::on_fps()
{
	auto fps = static_cast<CSliderUI*>(m_PaintManager.FindControl(L"fps")); assert(fps);
	if (!fps)return;
	auto val = fps->GetValue();
	JLOG_INFO("fps changed to {}", val);
}

void CDuiSettingsDlg::on_rec_time()
{
	auto rec_time = static_cast<CSliderUI*>(m_PaintManager.FindControl(L"rec_time")); assert(rec_time);
	if (!rec_time) { return; }
	auto val = rec_time->GetValue();
	JLOG_INFO("record time changed to {}", val);
	auto rec_time_text = static_cast<CLabelUI*>(m_PaintManager.FindControl(L"rec_time_text")); assert(rec_time_text);
	if (rec_time_text) {
		if (val == 0) {
			rec_time_text->SetText(L"max");
		} else {
			rec_time_text->SetText((std::to_wstring(val) + L"min").c_str());
		}
	}
}
