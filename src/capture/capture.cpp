#include <boost/program_options.hpp>
#include "KinectCapture.h"
#include <iostream>

namespace po = boost::program_options;
using namespace Speckle;

bool processCommandLine(int argc, char** argv, KinectCapture::Options & options) {
	po::options_description visible;
	int res;
	std::string mode;

	visible.add_options()
		("help",
			"Show help message and exit")
		("res", po::value<int>(&res),
		 	"The resolution, specified as a width in pixels. "
			"May be 320, 640 or 1280.")
		("mode", po::value<std::string>(&mode),
		 	"The capture mode, which may be:\n"
			"rgb: Bayer data converted to RGB by libfreenect.\n"
			"bayer: Raw bayer data, stored as DNG.\n"
			"ir8: IR data saved as 8-bit greyscale.\n"
			"ir10: IR data saved in 10-bit packed form.\n"
			"ir16: IR data zero-padded to 16 bits per sample." )
		("ir-brightness", po::value<int>(&options.brightness),
		 	"The output power of the IR projector, between 0 and 50.")
		("output,o", po::value<std::string>(&options.fileName),
		 	"The output filename. Please give it a .tif or .dng extension.")
		;

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv)
			.options(visible)
			.run(), vm);
	po::notify(vm);

	if (vm.count("help")) {
		std::cout << "Usage: " << (argc >= 1 ? argv[0] : "speckle-capture")
			<< " [options]\n"
			<< "Accepted options are:\n"
			<< visible;
		return false;
	}

	if (vm.count("res")) {
		switch (res) {
			case 320:
				options.resolution = FREENECT_RESOLUTION_LOW;
				break;
			case 640:
				options.resolution = FREENECT_RESOLUTION_MEDIUM;
				break;
			case 1280:
				options.resolution = FREENECT_RESOLUTION_HIGH;
				break;
			default:
				std::cout << "Invalid resolution, must be 320, 640 or 1280\n";
				return false;
		}
	}

	if (vm.count("mode")) {
		if (mode == "rgb") {
			options.mode = FREENECT_VIDEO_RGB;
		} else if (mode == "bayer") {
			options.mode = FREENECT_VIDEO_BAYER;
		} else if (mode == "ir8") {
			options.mode = FREENECT_VIDEO_IR_8BIT;
		} else if (mode == "ir10") {
			options.mode = FREENECT_VIDEO_IR_10BIT_PACKED;
		} else if (mode == "ir16") {
			options.mode = FREENECT_VIDEO_IR_10BIT;
		} else {
			std::cout << "Unknown mode \"" << mode << "\"\n";
			return false;
		}
	}

	if (vm.count("ir-brightness")) {
		if (options.brightness < 0) {
			options.brightness = 0;
		} else if (options.brightness > 50) {
			options.brightness = 50;
		}
	}

	if (!vm.count("output")) {
		std::cout << "The -o option is required\n";
		return false;
	}

	if (freenect_find_video_mode(options.resolution, options.mode).is_valid == 0) {
		std::cout << "Unable to set that combination of resolution and mode\n";
		return false;
	}

	return true;
}

int main(int argc, char** argv) {
	KinectCapture::Options options;
	if (!processCommandLine(argc, argv, options)) {
		return 1;
	}

	KinectCapture kc(options);
	if (kc.capture()) {
		return 0;
	} else {
		return 1;
	}
}
