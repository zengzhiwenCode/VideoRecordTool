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
#define sel_elif(val) else if (name == val) 

#define get_ctrl(type, name) auto name = static_cast<type*>(m_PaintManager.FindControl(utf8::a2w(#name).c_str())); assert(name);

		//auto video_min = static_cast<CLabelUI*>(m_PaintManager.FindControl(L"video_min")); assert(video_min);
		auto video_val = static_cast<CLabelUI*>(m_PaintManager.FindControl(L"video_val")); assert(video_val);
		auto video_max = static_cast<CLabelUI*>(m_PaintManager.FindControl(L"video_max")); assert(video_max);
		auto video_auto = static_cast<COptionUI*>(m_PaintManager.FindControl(L"video_auto")); assert(video_auto);
		auto video_slider = static_cast<CSliderUI*>(m_PaintManager.FindControl(L"video_slider")); assert(video_slider);
		auto video_inc = static_cast<CButtonUI*>(m_PaintManager.FindControl(L"video_inc")); assert(video_inc);
		auto video_dec = static_cast<CButtonUI*>(m_PaintManager.FindControl(L"video_dec")); assert(video_dec);
		get_ctrl(CLabelUI, video_min);

		auto vamp = cfg->get_procamp();

#define set_video_min_text(vname) \
		if (video_min) { \
			video_min->SetText(std::to_wstring(vamp.vname.min_).c_str()); \
		}

#define set_video_val_text(vname) \
		if (video_val) { \
			video_val->SetText(std::to_wstring(vamp.vname.val_).c_str()); \
		}

#define set_video_max_text(vname) \
		if (video_max) { \
			video_max->SetText(std::to_wstring(vamp.vname.max_).c_str()); \
		}

#define adjust_video_dec(vname) \
		if (video_dec) { \
			video_dec->SetEnabled(vamp.vname.valid_ > 0); \
		}

#define adjust_video_inc(vname) \
		if (video_inc) { \
			video_inc->SetEnabled(vamp.vname.valid_ > 0); \
		}

#define adjust_video_slider(vname) \
		if (video_slider) { \
			video_slider->SetMinValue(vamp.vname.min_); \
			video_slider->SetMaxValue(vamp.vname.max_); \
			video_slider->SetChangeStep(vamp.vname.step_); \
			video_slider->SetValue(vamp.vname.val_); \
			video_slider->SetEnabled(vamp.vname.valid_ > 0); \
		}

#define adjust_video_auto(vname) \
		if (video_auto) { \
			video_auto->SetEnabled(vamp.vname.flags_ == VideoProcAmp_Flags_Auto || vamp.vname.flags_ == VideoProcAmp_Flags_Manual); \
			video_auto->Selected(vamp.vname.flags_ == VideoProcAmp_Flags_Auto); \
		} 

		// adjust_video_auto(vname); 


#define case_video(vname, p) \
		else if (name == #vname) { \
			set_video_min_text(vname); \
			set_video_val_text(vname); \
			set_video_max_text(vname); \
			adjust_video_dec(vname); \
			adjust_video_inc(vname); \
			adjust_video_slider(vname); \
			pvideo_ = p; \
		}


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

		case_video(brightness, VideoProcAmp_Brightness)
		case_video(contrast, VideoProcAmp_Contrast)
		case_video(hue, VideoProcAmp_Hue)
		case_video(saturation, VideoProcAmp_Saturation)
		case_video(sharpness, VideoProcAmp_Sharpness)
		case_video(gamma, VideoProcAmp_Gamma)
		case_video(white_balance, VideoProcAmp_WhiteBalance)
		case_video(backlight, VideoProcAmp_BacklightCompensation)
		case_video(gain, VideoProcAmp_Gain)

		

#undef sel_elif

	} else if (type == "valuechanged") {
		if (name == "fps") {
			on_fps();
		} else if (name == "rec_time") {
			on_rec_time();
		} else if (name == "video_slider") {
			on_video_slider();
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

#define case_slider_dec(sname, control, handler) \
	else if (name == sname) { \
		auto control = static_cast<CSliderUI*>(m_PaintManager.FindControl(utf8::a2w(#control).c_str())); assert(control); \
		if (control) { \
			auto val = control->GetValue(); \
			if (val > control->GetMinValue()) { \
				control->SetValue(val - control->GetChangeStep()); \
				handler(); \
			} \
		} \
	}

#define case_slider_inc(sname, control, handler) \
	else if (name == sname) { \
		auto control = static_cast<CSliderUI*>(m_PaintManager.FindControl(utf8::a2w(#control).c_str())); assert(control); \
		if (control) { \
			auto val = control->GetValue(); \
			if (val < control->GetMaxValue()) { \
				control->SetValue(val + control->GetChangeStep()); \
				handler(); \
			} \
		} \
	}



	case_slider_dec("rec_time_dec", rec_time, on_rec_time)
	case_slider_inc("rec_time_inc", rec_time, on_rec_time)
	case_slider_dec("video_dec", video_slider, on_video_slider)
	case_slider_inc("video_inc", video_slider, on_video_slider)
	

	if (name == "reset_video") {
		if (maindlg->do_reset_video()) {
			//(static_cast<COptionUI*>(msg.pSender))->Selected(false);
			//on_video_slider();
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

void CDuiSettingsDlg::on_video_slider()
{
	auto video_slider = static_cast<CSliderUI*>(m_PaintManager.FindControl(L"video_slider")); assert(video_slider);
	if (!video_slider)return;
	int val = video_slider->GetValue();

	auto maindlg = static_cast<CvrmfcDlg*>(AfxGetApp()->GetMainWnd()); assert(maindlg);
	if (maindlg && maindlg->do_update_video(pvideo_, val)) {
		auto video_val = static_cast<CSliderUI*>(m_PaintManager.FindControl(L"video_val")); assert(video_val);
		if (!video_val)return;
		video_val->SetText(std::to_wstring(val).c_str());
	}
}
