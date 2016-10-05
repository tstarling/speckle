#include "ComputePipeline.h"

namespace Speckle {

ComputePipeline::ComputePipeline(const Options & options)
	: m_options(options),
	m_unpack(m_options.frameSize, m_options.bitsPerPixel),
	m_spatialWindow(m_options.spatialWindow, m_options.width),
	m_correlationTime(m_options.correlationTableSize, m_options.beta),
	m_visualize(m_options.minX)
{}

void ComputePipeline::writeFrame(void *data, size_t length, cv::Mat & output, int format) {
	if (length != m_options.frameSize) {
		throw std::runtime_error("Invalid frame length");
	}
	if (format != CV_8UC3 && format != CV_8UC4) {
		throw std::runtime_error("Invalid output format");
	}
	output.create(m_options.height, m_options.width, format);

	Mat3b * mat3;
	Mat4b * mat4;

	if (format == CV_8UC3) {
		mat3 = (Mat3b*)&output;
	} else {
		mat4 = (Mat4b*)&output;
	}

	m_unpack.startFrame(data);
	m_spatialWindow.startFrame();

	ComputePos pos;

	for (pos.y = 0; pos.y < m_options.height; pos.y++) {
		for (pos.x = 0; pos.x < m_options.width; pos.x++) {
			pos.outX = pos.outY = -1;
			int luminance = m_unpack.compute(pos);

			double kSq = m_spatialWindow.compute(pos, luminance);
			if (pos.outX == -1) {
				continue;
			}
			double x = m_correlationTime.compute(pos, kSq);
			cv::Vec3b c = m_visualize.compute(pos, x);
			if (format == CV_8UC4) {
				mat4->at<cv::Vec4b>(pos.outY, pos.outX) = cv::Vec4b(
					c[0], c[1], c[2], 0xff);
			} else {
				mat3->at<cv::Vec3b>(pos.outY, pos.outX) = c;
			}
		}
	}
}

} //namespace
