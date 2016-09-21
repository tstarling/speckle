#include "ComputePipeline.h"

namespace Speckle {

ComputePipeline::ComputePipeline(const Options & options, Mat3b & output)
	: m_options(options),
	m_unpack(m_options.frameSize, m_options.bitsPerPixel),
	m_spatialWindow(m_options.spatialWindow, m_options.width),
	m_correlationTime(m_options.correlationTableSize, m_options.baselineCorrelationTime),
	m_visualize(output, m_options.width, m_options.height, m_options.alpha)
{}

void ComputePipeline::writeFrame(void *data, size_t length) {
	if (length != m_options.frameSize) {
		throw std::runtime_error("Invalid frame length");
	}

	m_unpack.startFrame(data);
	m_spatialWindow.startFrame();

	ComputePos pos;

	for (pos.y = 0; pos.y < m_options.height; pos.y++) {
		for (pos.x = 0; pos.x < m_options.width; pos.x++) {
			/*
			if (pos.x > 10) {
				continue;
			}
			if (pos.y > 10) {
				break;
			}
			*/

			pos.outX = pos.outY = -1;
			int luminance = m_unpack.compute(pos);
			int lum8 = luminance >> (m_options.bitsPerPixel - 8);

			m_visualize.background(pos, cv::Vec3b(lum8, lum8, lum8));

			double kSq = m_spatialWindow.compute(pos, luminance);
			if (pos.outX == -1) {
				continue;
			}
			double x = m_correlationTime.compute(pos, kSq);
			m_visualize.foreground(pos, x);
		}
	}
}

} //namespace
