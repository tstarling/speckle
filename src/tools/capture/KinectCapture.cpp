#include "KinectCapture.h"
#include <tiffio.h>
#include <iostream>

namespace Speckle {

bool KinectCapture::capture() {
	freenect_context *f_ctx;
	freenect_device *f_dev;

	if (freenect_init(&f_ctx, NULL) < 0) {
		std::cerr << "Error: freenect_init() failed\n";
		return false;
	}

	freenect_set_log_level(f_ctx, FREENECT_LOG_SPEW);
	freenect_set_log_callback(f_ctx, LogCallback);

	freenect_select_subdevices(f_ctx, FREENECT_DEVICE_CAMERA);

	if (freenect_num_devices(f_ctx) < 1) {
		std::cerr << "Error: no Kinect devices found\n";
		return false;
	}

	if (freenect_open_device(f_ctx, &f_dev, 0) < 0) {
		std::cerr << "Error: could not open device\n";
		return false;
	}

	// FIXME: it's not possible to set the brightness in high-resolution mode.
	// It is overridden when the video starts, and then attempts to set it
	// after that respond with an error.
	sendBrightness(f_dev);

	freenect_set_video_callback(f_dev, VideoCallback);
	freenect_set_video_mode(f_dev, freenect_find_video_mode(m_options.resolution, m_options.mode));
	freenect_set_user(f_dev, (void*)this);
	freenect_start_video(f_dev);

	int res;
	while (!m_done && (res = freenect_process_events(f_ctx)) >= 0);

	freenect_stop_video(f_dev);
	freenect_close_device(f_dev);
	freenect_shutdown(f_ctx);

	return m_success;
}

void KinectCapture::sendBrightness(freenect_device *f_dev) {
	if (m_options.mode == FREENECT_VIDEO_IR_8BIT
			|| m_options.mode == FREENECT_VIDEO_IR_10BIT_PACKED
			|| m_options.mode == FREENECT_VIDEO_IR_10BIT)
	{
		freenect_set_ir_brightness(f_dev, (uint16_t)m_options.brightness);
	}
}

void KinectCapture::LogCallback(freenect_context *dev, freenect_loglevel level, const char *msg) {
	std::cerr << msg;
}

void KinectCapture::VideoCallback(freenect_device *dev, void *data, uint32_t timestamp) {
	KinectCapture & capture = *(KinectCapture*)freenect_get_user(dev);
	capture.processFrame(dev, data, timestamp);
}

void KinectCapture::processFrame(freenect_device *dev, void *data, uint32_t timestamp) {
	// Discard first frame
	// The first frame in high-resolution mode contains garbage. It is usually
	// discarded anyway due to a second bug (the frame is slightly too large),
	// but we'll take the frame after that in case that bug is ever fixed.
	if (m_frameIndex++ < 1) {
		return;
	}

	m_done = true;

	TIFF *tif = TIFFOpen(m_options.fileName.c_str(), "w");
	if (!tif) {
		std::cerr << "Unable to open output file\n";
		return;
	}

	freenect_frame_mode frameMode = freenect_get_current_video_mode(dev);
	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, frameMode.width);
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, frameMode.height);

	TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, frameMode.height);
	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
	TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE);
	TIFFSetField(tif, TIFFTAG_XRESOLUTION, 1.0);
	TIFFSetField(tif, TIFFTAG_YRESOLUTION, 1.0);

	size_t bitsPerPixel;

	switch (m_options.mode) {
		case FREENECT_VIDEO_RGB:
			TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
			TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
			TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
			TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
			bitsPerPixel = 24;
			break;
		case FREENECT_VIDEO_BAYER:
			// Let's pretend to be a DNG. We don't provide a baseline thumbnail
			// but we do most other things correctly. These files can be read by
			// ufraw.
			static uint8_t version[] = {1, 4, 0, 0};
			TIFFSetField(tif, TIFFTAG_DNGVERSION, version);
			TIFFSetField(tif, TIFFTAG_SUBFILETYPE, 0);
			TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_CFA);
			TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
			TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);

			// DNG does not allow LZW compression
			TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

			static uint16_t patternDim[] = {2, 2};
			TIFFSetField(tif, TIFFTAG_CFAREPEATPATTERNDIM, patternDim);
			static uint8_t pattern[] = {1, 0, 2, 1};
			TIFFSetField(tif, TIFFTAG_CFAPATTERN, pattern);
			bitsPerPixel = 8;
			break;
		case FREENECT_VIDEO_IR_8BIT:
			TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
			TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
			TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
			TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
			bitsPerPixel = 8;
			break;
		case FREENECT_VIDEO_IR_10BIT_PACKED:
			TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
			TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
			TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 10);

			// LZW is not helpful for this format
			TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

			bitsPerPixel = 10;
			break;
		case FREENECT_VIDEO_IR_10BIT:
			TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
			TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
			TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 16);
			TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
			bitsPerPixel = 16;
			break;
		default:
			std::cerr << "Invalid mode\n";
			TIFFClose(tif);
			return;
	}

	TIFFSetField(tif, TIFFTAG_MAKE, "Microsoft");
	TIFFSetField(tif, TIFFTAG_MODEL, "Kinect");
	TIFFSetField(tif, TIFFTAG_SOFTWARE, "libspeckle");

	size_t size = bitsPerPixel * frameMode.width * frameMode.height / 8;

	if (TIFFWriteEncodedStrip(tif, 0, data, size) < 0) {
		TIFFClose(tif);
		return;
	}

	TIFFClose(tif);

	m_success = true;
}

} // namespace
