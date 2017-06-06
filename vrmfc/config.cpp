#include "stdafx.h"
#include "config.h"
#include <fstream>
#include "jsoncpp/json.h"

namespace {

auto keyVid = "vid";
auto keyAid = "aid";

auto secVideo = "video";
	auto keyWidth = "width";
	auto keyHeight = "height";

auto secSerial = "serial";
	auto keyPort = "port";
	auto keyBaudrate = "baudrate";

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

		_port = value[secSerial][keyPort].asString();
		_baudrate = value[secSerial][keyBaudrate].asInt();

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

	value[secSerial][keyPort] = _port;
	value[secSerial][keyBaudrate] = _baudrate;

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

	// serial
	_port = "COM1";
	_baudrate = 9600;
}
