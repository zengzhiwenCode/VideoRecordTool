#include "stdafx.h"
#include "config.h"
#include <fstream>
#include "jsoncpp/json.h"

#include "duilib.h"



namespace {

auto keyVid = "vid";
auto keyAid = "aid";

auto secVideo = "video";
	auto keyWidth = "width";
	auto keyHeight = "height";
	auto keyRoot = "root";
	auto keyType = "type";
	auto keyTime = "rectime";

	auto keyValid = "valid";
	auto keyValue = "value";

	auto segProcamp = "procamp";
		auto keyBacklight = "backlight";
		auto keyBrightness = "brightness";
		auto keyContrast = "contrast";
		auto keyGain = "gain";
		auto keyGamma = "gamma";
		auto keyHue = "hue";
		auto keySaturation = "saturation";
		auto keySharpness = "sharpness";
		auto keyWhiteBalance = "white_balance";

	auto segCamera = "camera_settins";
		auto keyExposure = "exposure";
		auto keyFocus = "focus";
		auto keyIris = "iris";
		auto keyPan = "pan";
		auto keyRoll = "roll";
		auto keyTilt = "tilt";
		auto keyZoom = "zoom";

auto secSerial = "serial";
	auto keyPort = "port";
	auto keyBaudrate = "baudrate";

auto keyLang = "language";


}

config::config()
{
	cfg_file_ = get_exe_path_a() + "\\cfg.json";
	if (!load()) {
		init();
		save();
	}
}

bool config::load()
{
	std::ifstream in(cfg_file_);
	if (!in) { return false; }

	Json::Value value;
	Json::Reader reader;
	if (reader.parse(in, value)) {
		_aidx = value[keyAid].asInt();
		_vidx = value[keyVid].asInt();

		_video_w = value[secVideo][keyWidth].asInt();
		_video_h = value[secVideo][keyHeight].asInt();
		//_root = value[secVideo][keyRoot].asString();
		if (_root.empty() || !std::experimental::filesystem::is_directory(_root) || !std::experimental::filesystem::exists(_root)) {
			init_root();
		}
		_vtype = value[secVideo][keyType].asString();
		_max_rec_minutes = value[secVideo][keyTime].asInt();

		/*_procamp.backlight.val_ = value[secVideo][segProcamp][keyBacklight][keyValue].asInt();
		_procamp.brightness.val_ = value[secVideo][segProcamp][keyBrightness][keyValue].asInt();
		_procamp.contrast.val_ = value[secVideo][segProcamp][keyContrast][keyValue].asInt();
		_procamp.gain.val_ = value[secVideo][segProcamp][keyGain][keyValue].asInt();
		_procamp.gamma.val_ = value[secVideo][segProcamp][keyGamma][keyValue].asInt();
		_procamp.hue.val_ = value[secVideo][segProcamp][keyHue][keyValue].asInt();
		_procamp.saturation.val_ = value[secVideo][segProcamp][keySaturation][keyValue].asInt();
		_procamp.sharpness.val_ = value[secVideo][segProcamp][keySharpness][keyValue].asInt();
		_procamp.white_balance.val_ = value[secVideo][segProcamp][keyWhiteBalance][keyValue].asInt();

		_procamp.backlight.valid_ = value[secVideo][segProcamp][keyBacklight][keyValid].asInt();
		_procamp.brightness.valid_ = value[secVideo][segProcamp][keyBrightness][keyValid].asInt();
		_procamp.contrast.valid_ = value[secVideo][segProcamp][keyContrast][keyValid].asInt();
		_procamp.gain.valid_ = value[secVideo][segProcamp][keyGain][keyValid].asInt();
		_procamp.gamma.valid_ = value[secVideo][segProcamp][keyGamma][keyValid].asInt();
		_procamp.hue.valid_ = value[secVideo][segProcamp][keyHue][keyValid].asInt();
		_procamp.saturation.valid_ = value[secVideo][segProcamp][keySaturation][keyValid].asInt();
		_procamp.sharpness.valid_ = value[secVideo][segProcamp][keySharpness][keyValid].asInt();
		_procamp.white_balance.valid_ = value[secVideo][segProcamp][keyWhiteBalance][keyValid].asInt();

		_camera.exposure.val_ = value[secVideo][segCamera][keyExposure][keyValue].asInt();
		_camera.focus.val_ = value[secVideo][segCamera][keyFocus][keyValue].asInt();
		_camera.iris.val_ = value[secVideo][segCamera][keyIris][keyValue].asInt();
		_camera.pan.val_ = value[secVideo][segCamera][keyPan][keyValue].asInt();
		_camera.roll.val_ = value[secVideo][segCamera][keyRoll][keyValue].asInt();
		_camera.tilt.val_ = value[secVideo][segCamera][keyTilt][keyValue].asInt();
		_camera.zoom.val_ = value[secVideo][segCamera][keyZoom][keyValue].asInt();

		_camera.exposure.valid_ = value[secVideo][segCamera][keyExposure][keyValid].asInt();
		_camera.focus.valid_ = value[secVideo][segCamera][keyFocus][keyValid].asInt();
		_camera.iris.valid_ = value[secVideo][segCamera][keyIris][keyValid].asInt();
		_camera.pan.valid_ = value[secVideo][segCamera][keyPan][keyValid].asInt();
		_camera.roll.valid_ = value[secVideo][segCamera][keyRoll][keyValid].asInt();
		_camera.tilt.valid_ = value[secVideo][segCamera][keyTilt][keyValid].asInt();
		_camera.zoom.valid_ = value[secVideo][segCamera][keyZoom][keyValid].asInt();*/


		_port = value[secSerial][keyPort].asString();
		_baudrate = value[secSerial][keyBaudrate].asInt();

		_lang = value[keyLang].asString();
		if (_lang != "en") {
			_lang = "zh_CN";
		}

		return true;
	}

	return false;
}

bool config::save()
{
	std::ofstream out(cfg_file_);
	if (!out) { return false; }

	Json::Value value;
	value[keyAid] = _aidx;
	value[keyVid] = _vidx;

	value[secVideo][keyWidth] = _video_w;
	value[secVideo][keyHeight] = _video_h;
	//value[secVideo][keyRoot] = _root;
	value[secVideo][keyType] = _vtype;
	value[secVideo][keyTime] = _max_rec_minutes;

	/*value[secVideo][segProcamp][keyBacklight][keyValid] = _procamp.backlight.valid_;
	value[secVideo][segProcamp][keyBrightness][keyValid] = _procamp.brightness.valid_;
	value[secVideo][segProcamp][keyContrast][keyValid] = _procamp.contrast.valid_;
	value[secVideo][segProcamp][keyGain][keyValid] = _procamp.gain.valid_;
	value[secVideo][segProcamp][keyGamma][keyValid] = _procamp.gamma.valid_;
	value[secVideo][segProcamp][keyHue][keyValid] = _procamp.hue.valid_;
	value[secVideo][segProcamp][keySaturation][keyValid] = _procamp.saturation.valid_;
	value[secVideo][segProcamp][keySharpness][keyValid] = _procamp.sharpness.valid_;
	value[secVideo][segProcamp][keyWhiteBalance][keyValid] = _procamp.white_balance.valid_;

	value[secVideo][segProcamp][keyBacklight][keyValue] = _procamp.backlight.val_;
	value[secVideo][segProcamp][keyBrightness][keyValue] = _procamp.brightness.val_;
	value[secVideo][segProcamp][keyContrast][keyValue] = _procamp.contrast.val_;
	value[secVideo][segProcamp][keyGain][keyValue] = _procamp.gain.val_;
	value[secVideo][segProcamp][keyGamma][keyValue] = _procamp.gamma.val_;
	value[secVideo][segProcamp][keyHue][keyValue] = _procamp.hue.val_;
	value[secVideo][segProcamp][keySaturation][keyValue] = _procamp.saturation.val_;
	value[secVideo][segProcamp][keySharpness][keyValue] = _procamp.sharpness.val_;
	value[secVideo][segProcamp][keyWhiteBalance][keyValue] = _procamp.white_balance.val_;

	value[secVideo][segCamera][keyExposure][keyValid] = _camera.exposure.valid_;
	value[secVideo][segCamera][keyFocus][keyValid] = _camera.focus.valid_;
	value[secVideo][segCamera][keyIris][keyValid] = _camera.iris.valid_;
	value[secVideo][segCamera][keyPan][keyValid] = _camera.pan.valid_;
	value[secVideo][segCamera][keyRoll][keyValid] = _camera.roll.valid_;
	value[secVideo][segCamera][keyTilt][keyValid] = _camera.tilt.valid_;
	value[secVideo][segCamera][keyZoom][keyValid] = _camera.zoom.valid_;

	value[secVideo][segCamera][keyExposure][keyValue] = _camera.exposure.val_;
	value[secVideo][segCamera][keyFocus][keyValue] = _camera.focus.val_;
	value[secVideo][segCamera][keyIris][keyValue] = _camera.iris.val_;
	value[secVideo][segCamera][keyPan][keyValue] = _camera.pan.val_;
	value[secVideo][segCamera][keyRoll][keyValue] = _camera.roll.val_;
	value[secVideo][segCamera][keyTilt][keyValue] = _camera.tilt.val_;
	value[secVideo][segCamera][keyZoom][keyValue] = _camera.zoom.val_;*/


	value[secSerial][keyPort] = _port;
	value[secSerial][keyBaudrate] = _baudrate;

	value[keyLang] = _lang;

	Json::StyledStreamWriter writer;
	writer.write(out, value);

	return true;
}

void config::init()
{
	_vidx = 0;
	_aidx = 0;

	// video
	_video_w = 640;
	_video_h = 480;
	init_root();

	// serial
	_port = "COM1";
	_baudrate = 9600;

	// language
	_lang = "zh_CN";
}

void config::init_root()
{
	_root = utf8::u16_to_mbcs(get_exe_path() + L"\\" + utf8::a2w(VR_ROOT_FOLDER));
	fs::create_directory(_root);
	auto vpath = _root + "\\" + utf8::u16_to_mbcs(utf8::a2w(VR_VIDEO_FOLDER));
	fs::create_directory(vpath);
	auto cpath = _root + "\\" + utf8::u16_to_mbcs(utf8::a2w(VR_CAPTURE_FOLDER));
	fs::create_directory(cpath);
}

bool config::clear_root() const
{
	bool ok = true;
	std::error_code ec1, ec2, ec3;
	fs::path capture(get_capture_path());
	fs::remove_all(capture, ec1);
	fs::path record(get_video_path());
	fs::remove_all(record, ec2);
	fs::path thumb(get_thumb_path());
	fs::remove_all(thumb, ec3);

	if (ec1) {
		ok = false;
		JLOG_ERRO(ec1.message());
	}
	if (ec2) {
		ok = false;
		JLOG_ERRO(ec2.message());
	}
	if (ec3) {
		ok = false;
		JLOG_ERRO(ec3.message());
	}

	return ok;
}

std::string config::get_version() const
{
	return std::string("1.0.0.1");
}

std::string config::get_remainder_space() const
{
	auto path = fs::canonical(_root);
	auto root = path.root_path();
	fs::space_info si = fs::space(root);
	si.available;

	return format_space(si.available) + "/" + format_space(si.capacity);
}

std::string config::format_space(uintmax_t bytes)
{
	const int factor = 1024;
	uintmax_t KB = factor;
	uintmax_t MB = KB * factor;
	uintmax_t GB = MB * factor;
	uintmax_t TB = GB * factor;

	uintmax_t kb = bytes / factor;
	uintmax_t mb = kb / factor;
	uintmax_t gb = mb / factor;
	uintmax_t tb = gb / factor;

	std::string s;
	if (tb > 0) {
		gb -= tb * factor;
		gb = gb * 1000 / factor;
		gb /= 10;
		return std::to_string(tb) + "." + std::to_string(gb) + "T";
	} else if (gb > 0) {
		mb -= gb * factor;
		mb = mb * 1000 / factor;
		mb /= 10;
		return std::to_string(gb) + "." + std::to_string(mb) + "G";
	} else if (mb > 0) {
		kb -= mb * factor;
		kb = kb * 1000 / factor;
		kb /= 10;
		return std::to_string(mb) + "." + std::to_string(kb) + "M";
	} else if (kb > 0) {
		bytes -= kb * factor;
		bytes = bytes * 1000 / factor;
		bytes /= 10;
		return std::to_string(kb) + "." + std::to_string(bytes) + "K";
	} else {
		return std::to_string(bytes) + "B";
	}
}

void config::dump_amp() const
{
	_procamp.dump();
}

void config::dump_cam() const
{
	_camera.dump();
}

void config::dump_mi() const
{

}

std::string config::get_video_path() const
{
	auto p = _root + "\\" + utf8::u16_to_mbcs(utf8::a2w(VR_VIDEO_FOLDER));
	fs::create_directory(p);
	return p;
}

std::string config::get_capture_path() const
{
	auto p = _root + "\\" + utf8::u16_to_mbcs(utf8::a2w(VR_CAPTURE_FOLDER));
	fs::create_directory(p);
	return p;
}

std::string config::create_new_video_path() const
{
	auto s = now_to_string();
	s.erase(std::remove(s.begin(), s.end(), '-'), s.end());
	s.erase(std::remove(s.begin(), s.end(), ':'), s.end());
	std::replace(s.begin(), s.end(), ' ', '-');
	return get_video_path() + "\\" + s + VR_VIDEO_EXT;
}

std::string config::create_new_capture_path() const
{
	auto s = now_to_string();
	s.erase(std::remove(s.begin(), s.end(), '-'), s.end());
	s.erase(std::remove(s.begin(), s.end(), ':'), s.end());
	std::replace(s.begin(), s.end(), ' ', '-');
	return get_capture_path() + "\\" + s + VR_CAPTRUE_EXT;
}

std::string config::get_thumb_path() const
{
	auto p = _root + "\\" + VR_THUMBNAIL_FOLDER;
	fs::create_directory(p);
	return p;
}

std::string config::create_new_thumb_path(const std::string & stem)
{
	auto p = get_thumb_path() + "\\" + stem + VR_THUMBNAIL_EXT;
	return p;
}

std::string config::get_thumb_of_video(const std::string & vpath)
{
	auto thumb_path = create_new_thumb_path(fs::canonical(vpath).stem().string());
	if (fs::exists(thumb_path)) {
		return thumb_path;
	} else {
		using namespace cv;
		VideoCapture capture(vpath);
		if (capture.isOpened()) {
			Mat frame;
			capture >> frame; assert(!frame.empty());
			int c = frame.channels();			
			auto icon_play_path = utf8::u16_to_mbcs((CPaintManagerUI::GetResourcePath() + L"image\\play_128px.png").GetData());
			Mat icon = cv::imread(icon_play_path); assert(!icon.empty());
			c = icon.channels();
			Mat roi = frame(Range(frame.rows / 2 - icon.rows / 2, frame.rows / 2 + icon.rows / 2), 
							Range(frame.cols / 2 - icon.cols / 2, frame.cols / 2 + icon.cols / 2));
			auto alpha = 0.5;
			auto beta = 1.0 - alpha;
			addWeighted(roi, alpha, icon, beta, 0, roi);
			imwrite(thumb_path, frame);
			return thumb_path;
		}
	}

	return std::string();
}

std::string config::create_selected_pic_path(const std::string & stem)
{
	auto p = _root + "\\" + VR_SEL_PIC_FOLDER;
	fs::create_directory(p);
	p += "\\" + stem + VR_THUMBNAIL_EXT;
	return p;
}

std::string config::get_selected_pic(const std::string & path)
{
	auto sel_path = create_selected_pic_path(fs::canonical(path).stem().string());
	if (fs::exists(sel_path)) {
		return sel_path;
	} else {
		using namespace cv;
		Mat img = imread(path);
		if (img.data) {
			Mat mask(img.size(), img.type(), Scalar(255, 255, 255));
			Mat dst;
			addWeighted(img, 0.3, mask, 0.7, 0, dst);
			if (imwrite(sel_path, dst)) {
				return sel_path;
			}
		}
	}

	return std::string();
}

void media_info::dump() const
{
}

void vq::dump() const
{
	JLOG_INFO("-------------");
	JLOG_INFO("valid_ {}", valid_);
	JLOG_INFO("min_ {}", min_);
	JLOG_INFO("max_ {}", max_);
	JLOG_INFO("step_ {}", step_);
	JLOG_INFO("default_ {}", default_);
	JLOG_INFO("flags_ {}", flags_);
	JLOG_INFO("val_ {}", val_);
	JLOG_INFO("---------------------------");
}

void procamp::dump() const
{
	JLOG_INFO("backlight");
	backlight.dump();
	JLOG_INFO("brightness");
	brightness.dump();
	JLOG_INFO("contrast");
	contrast.dump();
	JLOG_INFO("gain");
	gain.dump();
	JLOG_INFO("gamma");
	gamma.dump();
	JLOG_INFO("hue");
	hue.dump();
	JLOG_INFO("saturation");
	saturation.dump();
	JLOG_INFO("sharpness");
	sharpness.dump();
	JLOG_INFO("white_balance");
	white_balance.dump();
}

void camera_set::dump() const
{
	JLOG_INFO("exposure");
	exposure.dump();
	JLOG_INFO("focus");
	focus.dump();
	JLOG_INFO("iris");
	iris.dump();
	JLOG_INFO("pan");
	pan.dump();
	JLOG_INFO("roll");
	roll.dump();
	JLOG_INFO("tilt");
	tilt.dump();
	JLOG_INFO("zoom");
	zoom.dump();
}
