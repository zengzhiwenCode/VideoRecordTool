#pragma once



namespace detail {


enum master_stream {
	no_master = -1,
	video_mater = 0,
	audio_master = 1,
};

inline master_stream int_to_master_stream(int value) {
	switch (value) {
	case master_stream::video_mater:
		return master_stream::video_mater;
		break;
	case master_stream::no_master:
		return master_stream::no_master;
		break;
	case master_stream::audio_master:
	default:
		return master_stream::audio_master;
		break;
	}
}

//struct device_info {
//	int idx;
//	std::wstring friendly_name;
//};
//
//typedef std::shared_ptr<device_info> device_ptr;
//typedef std::list<device_ptr> device_list;

struct device_info {
	std::wstring friendly_name;
	std::wstring display_name;
};

typedef std::map<int, device_info> device_info_map;



class capture_stuff
{
public:
	std::wstring title_;
	int w_, h_;

	capture_stuff();
	~capture_stuff();

	device_info_map get_video_devices() { GetDevices(); return video_moniker_friendly_name_map_; };
	device_info_map get_audio_devices() { GetDevices(); return audio_moniker_friendly_name_map_; };

	void set_video_window(HWND hWndVideo, HWND hWndStatus, RECT rcVideo);
	void ChooseDevices(const TCHAR *szVideo, const TCHAR *szAudio);
	BOOL StartPreview();
	// stop the preview graph
	BOOL StopPreview();
	// start the capture graph
	BOOL StartCapture();
	// stop the capture graph
	BOOL StopCapture();
	void OnFgNotify();
	void OnDeviceChange(WPARAM wParam, LPARAM lParam);
	const wchar_t* GetCaptureFile() const { return wszCaptureFile_; }
	BOOL IsCapturing() const { return fCapturing_; }
	BOOL IsPreviewing() const { return fPreviewing_; }

protected:
	void IMonRelease(IMoniker *&pm);
	BOOL MakeGraph();
	// Make a graph builder object we can use for capture graph building
	BOOL MakeSampleCaptureGraphBuilder();
	// build the capture graph
	BOOL BuildCaptureGraph();
	// Tear down everything downstream of a given filter
	void RemoveDownstream(IBaseFilter *pf);
	void GetDevices();
	BOOL BuildPreviewGraph();
	void TearDownGraph();
	// Check the devices we're currently using and make filters for them
	void ChooseDevices(IMoniker *pmVideo, IMoniker *pmAudio);
	static void __stdcall OnVideoWindowTimer(HWND, UINT, UINT_PTR, DWORD);
	// how many captured/dropped so far
	void UpdateStatus(BOOL fAllStats);
	// create the capture filters of the graph.  We need to keep them loaded from
	// the beginning, so we can set parameters on them and have them remembered
	BOOL InitCapFilters();
	// all done with the capture filters and the graph builder
	void FreeCapFilters();
	BOOL OpenFileDialog(HWND hWnd, LPTSTR pszName, DWORD cchName);
	BOOL SetCaptureFile(HWND hWnd);
	void SetAppCaption();

private:
	static std::map<int, capture_stuff*> self_map_for_timer_;
	WCHAR wszCaptureFile_[_MAX_PATH] = {};
	WORD wCapFileSize_ = 0;  // size in Meg
	ISampleCaptureGraphBuilder *pSampleCaptureGraphBuilder_ = nullptr;
	IVideoWindow *pVideoWindow_ = nullptr;
	IMediaEventEx *pMediaEventEx_ = nullptr;
	IAMDroppedFrames *pAMDroppedFrames_ = nullptr;
	IAMVideoCompression *pAMVideoCompression_ = nullptr;
	IAMVfwCaptureDialogs *pAMVfwCaptureDialogs_ = nullptr;
	IAMStreamConfig *pAMStreamConfigForAudio_ = nullptr;      // for audio cap
	IAMStreamConfig *pAMStreamConfigForVideo_ = nullptr;      // for video cap
	IBaseFilter *pBaseFilterRender_ = nullptr;
	IBaseFilter *pBaseFilterVideoCap_ = nullptr;
	IBaseFilter *pBaseFilterAudioCap_ = nullptr;
	IGraphBuilder *pGraphBuilder_ = nullptr;
	IFileSinkFilter *pFileSinkFilter_ = nullptr;
	IConfigAviMux *pConfigAviMux_ = nullptr;
	master_stream  master_stream_ = audio_master;
	BOOL fCaptureGraphBuilt = FALSE;
	BOOL fPreviewGraphBuilt_ = FALSE;
	BOOL fCapturing_ = FALSE;
	BOOL fPreviewing_ = FALSE;
	BOOL fMPEG2_ = FALSE;
	BOOL fCapAudio_ = TRUE;
	BOOL fCapClosedCaptioning_ = FALSE;
	BOOL fClosedCaptioningAvailable_ = FALSE;
	BOOL fCapAudioIsRelevant_ = FALSE;
	bool fDeviceMenuPopulated_ = false;
	IMoniker *pMonikerVideoMenus_[10];
	device_info_map video_moniker_friendly_name_map_ = {};
	IMoniker *pMonikerAudioMenus_[10];
	device_info_map audio_moniker_friendly_name_map_ = {};
	IMoniker *pMonikerVideo_ = nullptr;
	IMoniker *pMonikerAudio_ = nullptr;
	double FrameRate_ = 0.0;
	BOOL fWantPreview_ = 1;
	long lCapStartTime_ = 0L;
	long lCapStopTime_ = 0L;
	WCHAR FriendlyName_[120] = {};
	BOOL fUseTimeLimit_ = FALSE;
	BOOL fUseFrameRate_ = FALSE;
	DWORD dwTimeLimit_ = 0;
	int iFormatDialogPos_ = -1;			// Video Format...
	int iSourceDialogPos_ = -1;			// Video Source...
	int iDisplayDialogPos_ = -1;			// Video Display...
	int iVCapDialogPos_ = -1;			// Video Capture Filter...
	int iVCrossbarDialogPos_ = -1;		// Video Crossbar...
	int iTVTunerDialogPos_ = -1;			// TV Tuner...
	int iACapDialogPos_ = -1;			// Audio Capture Filter...
	int iACrossbarDialogPos_ = -1;		// Second Crossbar...
	int iTVAudioDialogPos_ = -1;			// TV Audio...
	int iVCapCapturePinDialogPos_ = -1;	// Video Capture Pin...
	int iVCapPreviewPinDialogPos_ = -1;	// Video Preview Pin...
	int iACapCapturePinDialogPos_ = -1;	// Audio Capture Pin...
	long lDroppedBase_ = 0L;
	long lNotBase_ = 0L;
	BOOL fPreviewFaked_ = FALSE;
	CCrossbar *pCrossbar_ = nullptr;
	int iVideoInputMenuPos_ = -1;
	LONG NumberOfVideoInputs_ = 0;
	HMENU hMenuPopup_ = nullptr;
	int iNumVCapDevices_ = 0;
	HWND hWndVideo_ = nullptr;
	HWND hWndStatus_ = nullptr;
	RECT rcVideo_ = {};
};








}
