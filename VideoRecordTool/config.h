#pragma once

class config : public dp::singleton<config>
{
protected:
	config();
	bool load();
	bool save();

private:
	std::string cfg_file_ = {};

	int vidx_ = 0;
	int aidx_ = 0;

public:
	int get_vid()const { return vidx_; }
	int get_aid()const { return aidx_; }

	void set_vid(int vid) { vidx_ = vid; save(); }
	void set_aid(int aid) { aidx_ = aid; save(); }

};

