#include "stdafx.h"
#include "DuiSettingsDlg.h"
#include "config.h"
#include "vrmfcDlg.h"
#include "vrmfc.h"

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

	// 分辨率/格式
	do {
		auto resolution = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(L"resolutions")); assert(resolution);
		if (!resolution) { break; }
		resolution->RemoveAll();
		resolution_btns_.clear();

		auto cap_mode = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(L"cap_mode")); assert(cap_mode);
		if (!cap_mode) { break; }
		cap_mode->RemoveAll();
		capmode_btns_.clear();

		auto mi = cfg->get_mi();
		for (const auto& i : mi) {
			auto opt = new COptionUI();
			capmode_btns_.push_back(opt);
			cap_mode->Add(opt);
			auto horz = new CHorizontalLayoutUI();
			horz->SetFixedHeight(5);
			cap_mode->Add(horz);

			auto name = utf8::a2w(i.first);
			opt->SetName((L"cap_mod_" + name).c_str());
			opt->SetText(name.c_str());
			opt->SetFont(0);
			opt->SetBkColor(0xFF3275EE);
			SIZE sz = { 15,15 };
			opt->SetBorderRound(sz);
			opt->SetGroup(L"cap_modes");

			if (i.first == cfg->get_vtype()) {
				opt->Selected(true); 
				opt->SetBkColor(0xFFD7E4FC);
			} else {
				opt->SetBkColor(0xFF3275EE);
			}
		}

		for (auto i : mi[cfg->get_vtype()].sizes) {
			auto opt = new COptionUI();
			resolution_btns_.push_back(opt);
			resolution->Add(opt);
			auto horz = new CHorizontalLayoutUI();
			horz->SetFixedHeight(5);
			resolution->Add(horz);

			opt->SetName((L"resolutions_" + std::to_wstring(i.first) + L"_" + std::to_wstring(i.second)).c_str());
			opt->SetText((std::to_wstring(i.first) + L"*" + std::to_wstring(i.second)).c_str());
			opt->SetFont(0);
			
			SIZE sz = { 15,15 };
			opt->SetBorderRound(sz);
			opt->SetGroup(L"resolutions");

			if (i.first == cfg->get_video_w() && i.second == cfg->get_video_h()) {
				opt->Selected(true);
				opt->SetBkColor(0xFFD7E4FC);
			} else {
				opt->SetBkColor(0xFF3275EE);
			}

			
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

	// 时间设置
	//SetTimer(m_hWnd, 1, 1000, nullptr);
	//GetLocalTime(&st_);
	time_ = COleDateTime::GetCurrentTime();
	update_time(time_);
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


		get_ctrl(CLabelUI, video_min);
		get_ctrl(CLabelUI, video_val);
		get_ctrl(CLabelUI, video_max);
		get_ctrl(COptionUI, video_auto);
		get_ctrl(CSliderUI, video_slider);
		get_ctrl(CButtonUI, video_inc);
		get_ctrl(CButtonUI, video_dec);

		get_ctrl(CLabelUI, camera_min);
		get_ctrl(CLabelUI, camera_val);
		get_ctrl(CLabelUI, camera_max);
		get_ctrl(COptionUI, camera_auto);
		get_ctrl(CSliderUI, camera_slider);
		get_ctrl(CButtonUI, camera_inc);
		get_ctrl(CButtonUI, camera_dec);

		auto vamp = cfg->get_procamp();
		auto camera = cfg->get_camera();

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
			cfg->notify_observers(0);
		} else if (name == "english") {
			cfg->set_lang("en"); SetThreadUILanguage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
			cfg->notify_observers(1);
		} else if (name == "time") {	// 时间
			options->SelectItem(5);
			time_ = COleDateTime::GetCurrentTime();
			update_time(time_);
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

		else if (name == "exposure") {
			camera_min->SetText(std::to_wstring(camera.exposure.min_).c_str());
			camera_val->SetText(std::to_wstring(camera.exposure.val_).c_str());
			camera_max->SetText(std::to_wstring(camera.exposure.max_).c_str());
			camera_slider->SetMinValue(camera.exposure.min_);
			camera_slider->SetMaxValue(camera.exposure.max_);
			camera_slider->SetChangeStep(camera.exposure.step_);
			camera_slider->SetValue(camera.exposure.val_);
			camera_dec->SetEnabled(camera.exposure.valid_ > 0);
			camera_inc->SetEnabled(camera.exposure.valid_ > 0);

		}

#define case_dt(v) else if (name == #v){ dt_ = v; }
		case_dt(year)
		case_dt(month)
		case_dt(day)
		case_dt(hour)
		case_dt(minute)

#undef sel_elif

	} else if (type == "valuechanged") {
		if (name == "fps") {
			on_fps();
		} else if (name == "rec_time") {
			on_rec_time();
		} else if (name == "video_slider") {
			on_video_slider();
		} else if (name == "camera_slider") {
			on_camera_slider();
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
	} else if (uMsg == WM_TIMER) {
		//update_time();
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
				
				auto wname = utf8::a2w(name);
				for(auto opt : capmode_btns_){
					if (opt) {
						if (opt->GetName().GetData() == wname) {
							opt->Selected(true);
							opt->SetBkColor(0xFFD7E4FC);
						} else {
							opt->Selected(false);
							opt->SetBkColor(0xFF3275EE);
						}
					}
				}
				
				// todo: maybe needless, update resolutions
			}
		}

		return;
	}

	// 分辨率
	std::string resolution = "resolutions_";
	if (name.find(resolution) != name.npos) {
		auto res = name.substr(resolution.length());
		auto pos = res.find('_');
		if (pos != res.npos) {
			int x = std::stoi(res.substr(0, pos));
			int y = std::stoi(res.substr(pos + 1));

			misz sz = { x,y };
			if (mi[cfg->get_vtype()].sizes.find(sz) != mi[cfg->get_vtype()].sizes.end()) {
				if (maindlg->do_update_resolution(sz)) {
					(static_cast<COptionUI*>(msg.pSender))->Selected(true);
					auto wname = utf8::a2w(name);
					for(auto opt : resolution_btns_){
						if (opt) {
							if (opt->GetName().GetData() == wname) {
								opt->Selected(true);
								opt->SetBkColor(0xFFD7E4FC);
							} else {
								opt->Selected(false);
								opt->SetBkColor(0xFF3275EE);
							}
						}

						
					}
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
	case_slider_dec("camera_dec", camera_slider, on_camera_slider)
	case_slider_inc("camera_inc", camera_slider, on_camera_slider)

	else if (name == "reset_video") {
		if (maindlg->do_reset_video()) {
			//(static_cast<COptionUI*>(msg.pSender))->Selected(false);
			//on_video_slider();
		}
	} else if (name == "reset_camera") {
		maindlg->do_reset_camera();
	} else if (name == "inc_time") {
		on_update_time(1);
	} else if (name == "dec_time") {
		on_update_time(-1);
	} else if (name == "apply_time") {
		on_apply_time();
	} else if (name == "reset_ok") {
		bool ok = true;
		if (!cfg->clear_root()) {
			ok = false;
		}

		maindlg->do_reset_video();
		maindlg->do_reset_camera();

		PostMessage(WM_CLOSE);
	} else if (name == "reset_cancel") {
		PostMessage(WM_CLOSE);
	}



	__super::OnClick(msg);
}

void CDuiSettingsDlg::on_update(const int & lang)
{
	tv vv = {
		{ L"resolution", trw(IDS_STRING_RESOLUTION) }, 
		{ L"record", trw(IDS_STRING_RECSET) },
		{ L"video", trw(IDS_STRING_VIDEOSET) },
		{ L"camera", trw(IDS_STRING_CAMSET) },
		{ L"language", trw(IDS_STRING_LANGSET) },
		{ L"time", trw(IDS_STRING_TIMESET) },
		{ L"recover", trw(IDS_STRING_RESET) },
		{ L"tab_resolution", trw(IDS_STRING_RESSET) },
		{ L"tab_record", trw(IDS_STRING_RECSET) },
			{ L"lable_rec_time", trw(IDS_STRING_RECTIME) },
		{ L"tab_video", trw(IDS_STRING_VIDEOSET) },
			{ L"brightness", trw(IDS_STRING_BRIGHTNESS) },
			{ L"contrast", trw(IDS_STRING_CONTRAST) },
			{ L"hue", trw(IDS_STRING_HUE) },
			{ L"saturation", trw(IDS_STRING_SATURATION) },
			{ L"sharpness", trw(IDS_STRING_SHARPNESS) },
			{ L"gamma", trw(IDS_STRING_GAMMA) },
			{ L"white_balance", trw(IDS_STRING_WB) },
			{ L"backlight", trw(IDS_STRING_BL) },
			{ L"gain", trw(IDS_STRING_GAIN) },
			{ L"reset_video", trw(IDS_STRING_RESET_TO_DEF) },
			{ L"video_auto", trw(IDS_STRING_AUTO) },
		{ L"tab_camera", trw(IDS_STRING_CAMSET) },
			{ L"zoom", trw(IDS_STRING_ZOOM) },
			{ L"focus", trw(IDS_STRING_FOCUS) },
			{ L"exposure", trw(IDS_STRING_EXPOSURE) },
			{ L"iris", trw(IDS_STRING_IRIS) },
			{ L"pan", trw(IDS_STRING_PAN) },
			{ L"tilt", trw(IDS_STRING_TILT) },
			{ L"roll", trw(IDS_STRING_ROLL) },
			{ L"reset_camera", trw(IDS_STRING_RESET_TO_DEF) },
			{ L"camera_auto", trw(IDS_STRING_AUTO) },
		{ L"tab_language", trw(IDS_STRING_LANGSET) },
		{ L"tab_time", trw(IDS_STRING_TIMESET) },
		{ L"apply_time", trw(IDS_STRING_APPLY) },
		{ L"tab_reset", trw(IDS_STRING_RESET) },
		{ L"reset_confirm", trw(IDS_STRING_RESET_CONFIRM) },
		{ L"reset_ok", trw(IDS_STRING_OK) },
		{ L"reset_cancel", trw(IDS_STRING_CANCEL) },
		{ L"closebtn", trw(IDS_STRING_CLOSE) },
	};

	for (auto v : vv) {
		auto ctrl = m_PaintManager.FindControl(v.first.c_str());
		if (ctrl) {
			ctrl->SetText(v.second.c_str());
		}
	}
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

void CDuiSettingsDlg::on_camera_slider()
{
	auto camera_slider = static_cast<CSliderUI*>(m_PaintManager.FindControl(L"camera_slider")); assert(camera_slider);
	if (!camera_slider)return;
	int val = camera_slider->GetValue();

	auto maindlg = static_cast<CvrmfcDlg*>(AfxGetApp()->GetMainWnd()); assert(maindlg);
	if (maindlg && maindlg->do_update_camera(pcamera_, val)) {
		auto camera_val = static_cast<CSliderUI*>(m_PaintManager.FindControl(L"camera_val")); assert(camera_val);
		if (!camera_val)return;
		camera_val->SetText(std::to_wstring(val).c_str());
	}

}

void CDuiSettingsDlg::on_update_time(int step)
{

	switch (dt_) {
	case CDuiSettingsDlg::year:
		if (step > 0) {
			time_ += COleDateTimeSpan(365, 0, 0, 0);
		} else {
			time_ -= COleDateTimeSpan(365, 0, 0, 0);
		}
		break;
	case CDuiSettingsDlg::month:
		if (step > 0) {
			time_ += COleDateTimeSpan(31, 0, 0, 0);
		} else {
			time_ -= COleDateTimeSpan(31, 0, 0, 0);
		}
		break;
	case CDuiSettingsDlg::day:
		if (step > 0) {
			time_ += COleDateTimeSpan(1, 0, 0, 0);
		} else {
			time_ -= COleDateTimeSpan(1, 0, 0, 0);
		}
		break;
	case CDuiSettingsDlg::hour:
		if (step > 0) {
			time_ += COleDateTimeSpan(0, 1, 0, 0);
		} else {
			time_ -= COleDateTimeSpan(0, 1, 0, 0);
		}
		break;
	case CDuiSettingsDlg::minute:
		if (step > 0) {
			time_ += COleDateTimeSpan(0, 0, 1, 0);
		} else {
			time_ -= COleDateTimeSpan(0, 0, 1, 0);
		}
		break;
	default:
		return;
		break;
	}

	update_time(time_);
}

void CDuiSettingsDlg::on_apply_time()
{
	SYSTEMTIME st = {};
	COleDateTime t(time_);
	t -= COleDateTimeSpan(0, 8, 0, 0);
	if (t.GetAsSystemTime(st)) {
		SetSystemTime(&st);
	}
}

void CDuiSettingsDlg::update_time(COleDateTime& st)
{
	do {
		auto cyear = static_cast<COptionUI*>(m_PaintManager.FindControl(L"year")); assert(cyear);
		auto cmonth = static_cast<COptionUI*>(m_PaintManager.FindControl(L"month")); assert(cmonth);
		auto cday = static_cast<COptionUI*>(m_PaintManager.FindControl(L"day")); assert(cday);
		auto chour = static_cast<COptionUI*>(m_PaintManager.FindControl(L"hour")); assert(chour);
		auto cmin = static_cast<COptionUI*>(m_PaintManager.FindControl(L"minute")); assert(cmin);

		cyear->SetText((std::to_wstring(st.GetYear()) + trw(IDS_STRING_YEAR)).c_str());
		cmonth->SetText((std::to_wstring(st.GetMonth()) + trw(IDS_STRING_MONTH)).c_str());
		cday->SetText((std::to_wstring(st.GetDay()) + trw(IDS_STRING_DAY)).c_str());
		chour->SetText((std::to_wstring(st.GetHour()) + trw(IDS_STRING_HOUR)).c_str());
		cmin->SetText((std::to_wstring(st.GetMinute()) + trw(IDS_STRING_MINUTE)).c_str());

	} while (false);
}
