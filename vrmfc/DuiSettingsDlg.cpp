#include "stdafx.h"
#include "DuiSettingsDlg.h"
#include "config.h"
#include "vrmfcDlg.h"

namespace {



}

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
	/*auto names = { L"320*240", L"640*360", L"640*480", L"920*720", L"1280*720" };
	for (auto rname : names) {
		auto opt = static_cast<COptionUI*>(m_PaintManager.FindControl(rname));
		if (opt) {
			opt->Selected(dst == rname);
		}
	}*/

	// 分辨率/格式
	do {
		auto resolution = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(L"resolutions")); assert(resolution);
		if (!resolution) { break; }
		resolution->RemoveAll();

		auto cap_mode = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(L"cap_mode")); assert(cap_mode);
		if (!cap_mode) { break; }
		cap_mode->RemoveAll();

		auto mi = cfg->get_mi();
		for (const auto& i : mi) {
			auto opt = new COptionUI();
			auto name = utf8::a2w(i.first);
			opt->SetName((L"cap_mod_" + name).c_str());
			opt->SetText(name.c_str());
			opt->SetFont(0);
			opt->SetBkColor(0xFF3275EE);
			SIZE sz = { 15,15 };
			opt->SetBorderRound(sz);
			opt->SetGroup(L"cap_mode");

			if (i.first == cfg->get_vtype()) {
				opt->Selected(true);
			}

			cap_mode->Add(opt);

			auto horz = new CHorizontalLayoutUI();
			horz->SetFixedHeight(5);
			cap_mode->Add(horz);
		}

		for (auto i : mi[cfg->get_vtype()].sizes) {
			auto opt = new COptionUI();
			auto name = std::to_wstring(i.first) + L"*" + std::to_wstring(i.second);
			opt->SetName((L"resolutions_" + name).c_str());
			opt->SetText(name.c_str());
			opt->SetFont(0);
			opt->SetBkColor(0xFF3275EE);
			SIZE sz = { 15,15 };
			opt->SetBorderRound(sz);
			opt->SetGroup(L"resolutions");

			if (i.first == cfg->get_video_w() && i.second == cfg->get_video_h()) {
				opt->Selected(true);
			}

			resolution->Add(opt);

			auto horz = new CHorizontalLayoutUI();
			horz->SetFixedHeight(5);
			resolution->Add(horz);
		}

	} while (false);

	// 录像选项
	do {
		auto rec_time = static_cast<CSliderUI*>(m_PaintManager.FindControl(L"rec_time")); assert(rec_time);
		if (!rec_time) { break; }
		rec_time->SetValue(cfg->get_max_rec_minutes());
		on_rec_time();
	} while (false);

	// 视频设置
	do {
		auto vamp = cfg->get_procamp();

		auto brightness = static_cast<COptionUI*>(m_PaintManager.FindControl(L"brightness"));
		if (brightness) {
			brightness->SetEnabled(vamp.brightness.valid_ > 0);
		}

#define adjust_video_btns(btnname) \
		auto btnname = static_cast<COptionUI*>(m_PaintManager.FindControl(utf8::a2w(#btnname).c_str())); \
		if (btnname) { \
			btnname->SetEnabled(vamp.btnname.valid_ > 0); \
		}

		adjust_video_btns(contrast);
		adjust_video_btns(hue);
		adjust_video_btns(saturation);
		adjust_video_btns(sharpness);
		adjust_video_btns(gamma);
		adjust_video_btns(white_balance);
		adjust_video_btns(backlight);
		adjust_video_btns(gain);

#undef adjust_video_btns
	} while (false);



	// 摄像机设置
	do {
		auto cam = cfg->get_camera();

		auto zoom = static_cast<COptionUI*>(m_PaintManager.FindControl(L"zoom"));
		if (zoom) {
			zoom->SetEnabled(cam.zoom.valid_ > 0);
		}

#define adjust_camera_btn(btname) \
		auto btname = static_cast<COptionUI*>(m_PaintManager.FindControl(utf8::a2w(#btname).c_str())); \
		if (btname) { \
			btname->SetEnabled(cam.btname.valid_ > 0); \
		} 

		adjust_camera_btn(focus);
		adjust_camera_btn(exposure);
		adjust_camera_btn(pan);
		adjust_camera_btn(tilt);
		adjust_camera_btn(roll);
		adjust_camera_btn(iris);

#undef adjust_camera_btn

	} while (false);

	// 语言设置
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
	range_log rl("CDuiSettingsDlg::Notify type=" + type + " name=" + name);
	CTabLayoutUI* options = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("options"))); assert(options);
	auto cfg = config::get_instance();
	if (type == "selectchanged") {
		

		if (name == "resolution") {		// 分辨率/格式设置
			options->SelectItem(0);
		} else if (name == "record") {  // 录像设置
			options->SelectItem(1);
		} else if (name == "video") {	// 视频
			options->SelectItem(2);	

		} else if (name == "camera") {	// 摄像机
			options->SelectItem(3);
		} else if (name == "language") { // 语言设置
			options->SelectItem(4);
		} else if (name == "chinese") {
			cfg->set_lang("zh_CN"); SetThreadUILanguage(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED));
		} else if (name == "english") {
			cfg->set_lang("en"); SetThreadUILanguage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
		} else if (name == "time") {	// 时间
			options->SelectItem(5);
		} else if (name == "recover") {	// 恢复
			options->SelectItem(6);
		}

#define sel_elif(val) else if (name == val) 

		auto video_min = static_cast<CLabelUI*>(m_PaintManager.FindControl(L"video_min")); assert(video_min);
		auto video_val = static_cast<CLabelUI*>(m_PaintManager.FindControl(L"video_min")); assert(video_min);
		auto video_max = static_cast<CLabelUI*>(m_PaintManager.FindControl(L"video_min")); assert(video_min);

		
		if (name == "brightness") {

		} sel_elif("contrast") {

		} sel_elif("hue") {

		} sel_elif("saturation") {

		} sel_elif("sharpness") {

		} sel_elif("gamma") {

		} sel_elif("white_balance") {

		} sel_elif("backlight") {

		} sel_elif("gain") {

		}

#undef sel_elif

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
	range_log rl("CDuiSettingsDlg::OnClick " + name);

	auto maindlg = static_cast<CvrmfcDlg*>(AfxGetApp()->GetMainWnd()); assert(maindlg);
	if (!maindlg) { return; }
	
	auto cfg = config::get_instance();
	auto mi = cfg->get_mi();

	// 格式
	std::string cap_mode = "cap_mod_";
	if (name.find(cap_mode) != name.npos) {
		auto mode = name.substr(cap_mode.length());
		if (mi.find(mode) != mi.end()) {
			if (maindlg->do_update_capmode(mode)) {
				(static_cast<COptionUI*>(msg.pSender))->Selected(true);
				// todo: maybe needless, update resolutions
			}
		}

		return;
	}

	// 分辨率
	std::string resolution = "resolutions_";
	if (name.find(resolution) != name.npos) {
		auto res = name.substr(resolution.length());
		auto pos = res.find('*');
		if (pos != res.npos) {
			int x = std::stoi(res.substr(0, pos));
			int y = std::stoi(res.substr(pos + 1));

			misz sz = { x,y };
			if (mi[cfg->get_vtype()].sizes.find(sz) != mi[cfg->get_vtype()].sizes.end()) {
				if (maindlg->do_update_resolution(sz)) {
					(static_cast<COptionUI*>(msg.pSender))->Selected(true);
				}
			}
		}
		
		return;
	}

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
	config::get_instance()->set_max_rec_minutes(val);
	auto rec_time_text = static_cast<CLabelUI*>(m_PaintManager.FindControl(L"rec_time_text")); assert(rec_time_text);
	if (rec_time_text) {
		if (val == 0) {
			rec_time_text->SetText(L"max");
		} else {
			rec_time_text->SetText((std::to_wstring(val) + L"min").c_str());
		}
	}
}
