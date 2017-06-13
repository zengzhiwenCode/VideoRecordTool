#include "stdafx.h"
#include "config.h"
#include <fstream>
#include "jsoncpp/json.h"
#include <filesystem>

namespace {

auto keyVid = "vid";
auto keyAid = "aid";

auto secVideo = "video";
	auto keyWidth = "width";
	auto keyHeight = "height";
	auto keyRoot = "root";

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
