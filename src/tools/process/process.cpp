#include <boost/program_options.hpp>
#include <iostream>
#include <tiffio.h>
#include <cstdlib>
#include <limits>
#include <vector>
#include <opencv2/highgui/highgui.hpp>
#include <cstdint>

#include "compute/ComputePipeline.h"

namespace po = boost::program_options;
using namespace Speckle;

bool processCommandLine(int argc, char** argv,
		std::string & input,
		std::string & output,
		ComputePipeline::Options & options)
{
	po::options_description visible;
	
	visible.add_options()
		("help",
		 	"Show help message and exit")
		("window", po::value<int>(&options.spatialWindow),
		 	"Spatial window size, should be an odd number of pixels (default 7)")
		("correlation-table-size", po::value<int>(&options.correlationTableSize),
		 	"Table size used for solving the correlation time equation (default 1024)")
		("baseline", po::value<double>(&options.baselineCorrelationTime),
		 	"Baseline correlation time")
		("alpha", po::value<double>(&options.alpha),
		 	"Blend speckle visualization with source image with this opacity value (default 1)")
		;

	po::options_description invisible;
	invisible.add_options()
		("source", po::value<std::string>(&input))
		("dest", po::value<std::string>(&output))
		;

	po::options_description allDesc;
	allDesc.add(visible).add(invisible);

	po::positional_options_description positionalDesc;
	positionalDesc
		.add("source", 1)
		.add("dest", 1)
		;

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv)
			.options(allDesc)
			.positional(positionalDesc)
			.run(), vm);
	po::notify(vm);

	if (vm.count("help")) {
		std::cout << "Usage: " << (argc >= 1 ? argv[0] : "process" )
			<< " [options] <source> <dest>\n"
			<< "Accepted options are:\n"
			<< visible;
		return false;
	}

	return true;
}

void getOrThrow(TIFF *tif, ttag_t tag, ...) {
	va_list args;
	va_start(args, tag);
	int ret = TIFFVGetField(tif, tag, args);
	if (!ret) {
		std::cerr << "Unable to read required TIFF tag\n";
		std::exit(1);
	}
	va_end(args);
}

int main(int argc, char **argv) {
	ComputePipeline::Options options;
	std::string inputName;
	std::string outputName;

	if (!processCommandLine(argc, argv, inputName, outputName, options)) {
		return 1;
	}

	TIFF *tiffInput = TIFFOpen(inputName.c_str(), "r");
	if (!tiffInput) {
		std::cerr << "Unable to open input file\n";
		return 1;
	}

	uint32_t width, height;
	uint16_t samplesPerPixel, bitsPerSample;

	getOrThrow(tiffInput, TIFFTAG_IMAGEWIDTH, &width);
	getOrThrow(tiffInput, TIFFTAG_IMAGELENGTH, &height);
	getOrThrow(tiffInput, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
	getOrThrow(tiffInput, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);

	if (samplesPerPixel != 1) {
		std::cerr << "Colour input images are not yet supported\n";
		return 1;
	}

	tsize_t lineSize = TIFFScanlineSize(tiffInput);

	if (height > std::numeric_limits<int>::max() / lineSize) {
		std::cerr << "Image too large\n";
		return 1;
	}

	std::vector<uint8_t> buffer(height * lineSize);

	for (uint32_t y = 0; y < height; y++) {
		if (1 != TIFFReadScanline(tiffInput, &(buffer[y * lineSize]), y, 0)) {
			std::cerr << "Error reading TIFF file\n";
			return 1;
		}
	}
	TIFFClose(tiffInput);

	options.width = width;
	options.height = height;
	options.bitsPerPixel = bitsPerSample;
	options.frameSize = height * width * bitsPerSample / 8;

	Mat3b result;

	ComputePipeline compute(options, result);
	compute.writeFrame(&(buffer[0]), options.frameSize);

	cv::imwrite(outputName, result);

	return 0;
}
