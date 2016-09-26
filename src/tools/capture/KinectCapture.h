#include <libfreenect.h>
#include <iostream>
#include <tiffio.h>

namespace Speckle {

class KinectCapture {
public:

	struct Options {
		Options()
			: resolution(FREENECT_RESOLUTION_MEDIUM),
			mode(FREENECT_VIDEO_RGB),
			brightness(10),
			frames(1),
			skip(1)
		{}

		freenect_resolution resolution;
		freenect_video_format mode;
		int brightness;
		std::string fileName;
		int frames;
		int skip;
	};

	KinectCapture(const Options & options)
		: m_options(options), m_done(false), m_success(false), m_frameIndex(0)
	{}

	bool capture();
private:
	static void VideoCallback(freenect_device *dev, void *data, uint32_t timestamp);
	static void LogCallback(freenect_context *dev, freenect_loglevel level, const char *msg);

	void processFrame(freenect_device *dev, void *data, uint32_t timestamp);
	void sendBrightness(freenect_device *f_dev);

	Options m_options;
	bool m_done;
	bool m_success;

	int m_frameIndex;
	TIFF * m_tif;
};

} // namespace
