
#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <opencv2/core/core.hpp>

#include "compute/SpatialWindow.h"
#include "compute/CorrelationTime.h"

struct TestError : public std::runtime_error {
	TestError(const char * msg)
		: std::runtime_error(msg)
	{}

	TestError(const std::string & msg)
		: std::runtime_error(msg)
	{}
};

template <class DataType>
cv::Mat readMatrix(std::istream & f, int cvType) {
	std::vector<DataType> data;
	std::string line;
	int width = 0;
	int height = 0;
	while (true) {
		std::getline(f, line);
		if (f.eof()) {
			break;
		} else if (!f.good()) {
			throw TestError("readMatrix: Unexpected file read error");
		}
		size_t tabPos = line.find("\t");
		if (tabPos == std::string::npos || tabPos == 0) {
			// End of matrix
			break;
		}
		height++;
		width = 0;
		std::istringstream lineStream(line);
		while (lineStream.good()) {
			DataType value = 0;
			lineStream >> value;
			if (!lineStream.fail()) {
				width++;
				data.push_back(value);
			}
		}
	}
	// All rows must be the same length
	if (size_t(height) * width != data.size()) {
		throw TestError("Invalid matrix, the rows are not all the same length");
	}
	return cv::Mat(data, true).reshape(1, height);
}

template <class T>
void assertEquals(T actual, T expected, const char * msg) {
	if (actual == expected) {
		return;
	}
	throw TestError(std::string("Failed assertion: \"") + msg + "\": expected " +
			std::to_string(expected) +
			", got " + std::to_string(actual));
}

void assertApproxEquals(double actual, double expected, double threshold = 1e-9) {
	double error = std::fabs(actual - expected);
	double relError = expected == 0. ? error : error / expected;
	if (relError > threshold) {
		throw TestError(std::string("Failed assertion: expected ") + std::to_string(expected) +
				", got " + std::to_string(actual) +
				", rel. error = " + std::to_string(relError));
	}
}

void assertRoughlyEquals(double actual, double expected) {
	assertApproxEquals(actual, expected, 1e-3);
}

bool testSpatialWindow(std::ifstream & f) {
	while (true) {
		int window = -1;
		std::string line;
		std::string testName;

		// Read attributes
		while (true) {
			std::getline(f, line);
			if (f.eof()) {
				return true;
			}
			if (!f.good()) {
				std::cout << "testSpatialWindow: Unexpected file read error\n";
				return false;
			}
			size_t tabPos = line.find("\t");
			if (tabPos == 0 || tabPos == std::string::npos) {
				break;
			}
			std::string attrName = line.substr(0, tabPos);
			size_t tabPos2 = line.find("\t", tabPos + 1);
			std::string attrValue = (tabPos2 == std::string::npos) ? line.substr(tabPos + 1)
				: line.substr(tabPos + 1, tabPos2 - tabPos - 1);
			if (attrName == "test") {
				testName = attrValue;
			} else if (attrName == "window") {
				window = std::stoi(attrValue);
			}
		}

		if (testName.empty()) {
			std::cout << "Missing test name\n";
			return false;
		}
		if (window == -1) {
			std::cout << "Missing window\n";
			return false;
		}

		std::cout << "Running test: " << testName << " ";

		// Read data

		cv::Mat input = readMatrix<uint16_t>(f, CV_16UC1);
		cv::Mat expected = readMatrix<double>(f, CV_64FC1);

		int offset = (window % 2) ? ((window - 1) / 2) : (window / 2);

		assertEquals(input.rows - window + 1, expected.rows, "rows");
		assertEquals(input.cols - window + 1, expected.cols, "cols");

		Speckle::SpatialWindow spatialWindow(window, input.cols);
		spatialWindow.startFrame();
		Speckle::ComputePos pos;
		for (pos.y = 0; pos.y < input.rows; pos.y++) {
			for (pos.x = 0; pos.x < input.cols; pos.x++) {
				pos.outX = pos.outY = -1;
				double value = spatialWindow.compute(pos, input.at<uint16_t>(pos.y, pos.x));
				if (pos.y < window - 1 || pos.x < window - 1) {
					assertEquals(pos.outY, -1, "outY");
					assertEquals(pos.outX, -1, "outX");
					continue;
				}
				assertEquals(pos.outY, pos.y - offset, "outX");
				assertEquals(pos.outX, pos.x - offset, "outY");
				double k = expected.at<double>(pos.y - window + 1, pos.x - window + 1);
				assertApproxEquals(value, k * k);
			}
		}

		std::cout << "OK\n";
	}
}

bool testCorrelationTime(std::ifstream & f) {
	// Header line
	std::string line;
	std::getline(f, line);

	cv::Mat data = readMatrix<double>(f, CV_64FC1);

	// Forwards
	std::cout << "Compute k^2 from x: ";
	for (int i = 0; i < data.rows; i++) {
		double x = data.at<double>(i, 0);
		double beta = data.at<double>(i, 1);
		double ksq = data.at<double>(i, 2);
		assertApproxEquals(Speckle::getKSquared(x), ksq / beta);
	}
	std::cout << "OK\n";

	// Inverse
	std::cout << "Compute x from k^2: ";
	for (int i = 0; i < data.rows; i++) {
		double x = data.at<double>(i, 0);
		double beta = data.at<double>(i, 1);
		double ksq = data.at<double>(i, 2);
		Speckle::CorrelationTime corr(1024, beta);
		Speckle::ComputePos pos;
		assertRoughlyEquals(corr.compute(pos, ksq), x);
	}
	std::cout << "OK\n";
	return true;
}

int main(int argc, char **argv) {
	if (argc < 3) {
		std::cout << "Usage: test <subcommand> <data-file>\n";
		return EXIT_FAILURE;
	}

	char * cmd = argv[1];
	std::ifstream file(argv[2]);
	if (!file.good()) {
		std::cout << "Unable to open input file\n";
		return EXIT_FAILURE;
	}

	bool success;
	try {
		if (!std::strcmp(cmd, "SpatialWindow")) {
			success = testSpatialWindow(file);
		} else if (!std::strcmp(cmd, "CorrelationTime")) {
			success = testCorrelationTime(file); 
		} else {
			std::cout << "Unrecognised command\n";
			success = false;
		}
	} catch (TestError & e) {
		std::cout << e.what() << "\n";
		success = false;
	}
	if (file.bad()) {
		std::cout << "Error reading from data file\n";
		success = false;
	}
	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}


