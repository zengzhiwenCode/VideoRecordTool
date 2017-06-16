#include "stdafx.h"
#include "config.h"
#include <fstream>
#include "jsoncpp/json.h"
#include <filesystem>
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
		_root = value[secVideo][keyRoot].asString();
		if (_root.empty() || !std::experimental::filesystem::is_directory(_root) || !std::experimental::filesystem::exists(_root)) {
			init_root();
		}
		_vtype = value[secVideo][keyType].asString();
		_max_rec_minutes = value[secVideo][keyTime].asInt();

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
	value[secVideo][keyRoot] = _root;
	value[secVideo][keyType] = _vtype;
	value[secVideo][keyTime] = _max_rec_minutes;

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
	_root = get_exe_path_a() + "\\" + VR_ROOT_FOLDER;
	CreateDirectoryA(_root.c_str(), nullptr);
	auto vpath = _root + "\\" + VR_VIDEO_FOLDER;
	CreateDirectoryA(vpath.c_str(), nullptr);
	auto cpath = _root + "\\" + VR_CAPTURE_FOLDER;
	CreateDirectoryA(cpath.c_str(), nullptr);
}

std::string config::get_video_path() const
{
	auto p = _root + "\\" + VR_VIDEO_FOLDER;
	CreateDirectoryA(p.c_str(), nullptr);
	return p;
}

std::string config::get_capture_path() const
{
	auto p = _root + "\\" + VR_CAPTURE_FOLDER;
	CreateDirectoryA(p.c_str(), nullptr);
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
	CreateDirectoryA(p.c_str(), nullptr);
	return p;
}

std::string config::create_new_thumb_path(const std::string & stem)
{
	auto p = get_thumb_path() + "\\" + stem + VR_THUMBNAIL_EXT;
	return p;
}

std::string config::get_thumb_of_video(const std::string & vpath)
{
	namespace fs = std::experimental::filesystem;
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
			//imshow("frame", frame);
			
			auto icon_play_path = utf8::w2a((CPaintManagerUI::GetResourcePath() + L"image\\play_128px.png").GetData());
			Mat icon = cv::imread(icon_play_path); assert(!icon.empty());
			//imshow("icon", icon);
			c = icon.channels();

			//Mat roi = frame(Range(frame.rows / 4, frame.rows * 3 / 4), Range(frame.cols / 4, frame.cols * 3 / 4));
			Mat roi = frame(Range(frame.rows / 2 - icon.rows / 2, frame.rows / 2 + icon.rows / 2), 
							Range(frame.cols / 2 - icon.cols / 2, frame.cols / 2 + icon.cols / 2));
			//imshow("roi", roi);

			//waitKey();
			auto alpha = 0.5;
			auto beta = 1.0 - alpha;
			addWeighted(roi, alpha, icon, beta, 0, roi);
			//bitwise_or(roi, icon, roi);

			// Now create a mask of logo and create its inverse mask also
			//Mat icon_gray;
			//cvtColor(icon, icon_gray, COLOR_BGR2GRAY);
			/*Mat mask;
			threshold(icon, mask, 30, 200, THRESH_BINARY);

			add(roi, icon, roi, mask);
*/

			////	ret, mask = cv2.threshold(img2gray, 10, 255, cv2.THRESH_BINARY)
			//Mat mask_inv;
			//bitwise_not(mask, mask_inv);
			//// Now black - out the area of logo in ROI
			////Mat roi_bg;
			//bitwise_and(roi, roi, roi, mask_inv);
			//// Take only region of logo from logo image.
			//Mat icon_fg;
			//bitwise_and(icon, icon, icon_fg, mask);
			//// Put logo in ROI and modify the main image
			////Mat roi_dst;
			//add(roi, icon_fg, roi);
			////roi = roi_dst;

			imwrite(thumb_path, frame);
			return thumb_path;
		}
	}

	return std::string();
}
