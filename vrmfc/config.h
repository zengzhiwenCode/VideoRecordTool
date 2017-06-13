#pragma once

#define VR_ROOT_FOLDER		"root"
#define VR_VIDEO_FOLDER		"video" 
#define VR_VIDEO_EXT		".avi"
#define VR_CAPTURE_FOLDER	"capture"
#define VR_CAPTRUE_EXT		".bmp"
#define VR_THUMBNAIL_FOLDER "thumb"
#define VR_THUMBNAIL_EXT	".png"

class config : public dp::singleton<config>
{
protected:
	config();
	bool load();
	bool save();
	void init();
	void init_root();

private:
	std::string cfg_file_ = {};

	int _vidx = 0;
	int _aidx = 0;

	// video
	int _video_w = 0;
	int _video_h = 0;
	std::string _root = {};

	// serial
	std::string _port = {};
	int _baudrate = 9600;

	// language 
	std::string _lang = {};

public:
	std::string get_video_path() const;
	std::string get_capture_path() const;
	std::string create_new_video_path() const;
	std::string create_new_capture_path() const;
	

#define declare_getter(type, val) type get##val() const { return val; }
#define declare_getter_int(val) declare_getter(int, val)

#define declare_setter(type, val) void set##val(const type& v) { if (val == v) return; val = v; save(); }
#define declare_setter_int(val) declare_setter(int, val)

#define declare_gs(type, val) declare_getter(type, val); declare_setter(type, val);
#define declare_gs_int(val) declare_gs(int, val)
#define declare_gs_string(val) declare_gs(std::string, val)



	declare_gs_int(_vidx);
	declare_gs_int(_aidx);
	declare_gs_int(_video_w);
	declare_gs_int(_video_h);
	declare_gs_string(_root);
	declare_gs_string(_port);
	declare_gs_int(_baudrate);
	declare_gs_string(_lang);


	
};

