#include "stdafx.h"
#include "config.h"
#include <fstream>
#include "jsoncpp/json.h"

namespace {

auto keyVid = "vid";
auto keyAid = "aid";

}

config::config()
{
	cfg_file_ = get_exe_path_a() + "\\cfg.json";
	if (!load()) {
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
		aidx_ = value[keyAid].asInt();
		vidx_ = value[keyVid].asInt();

		return true;
	}

	return false;
}

bool config::save()
{
	std::ofstream out(cfg_file_);
	if (!out) { return false; }

	Json::Value value;
	value[keyAid] = aidx_;
	value[keyVid] = vidx_;

	Json::StyledStreamWriter writer;
	writer.write(out, value);

	return true;
}
