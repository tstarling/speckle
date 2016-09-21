#include "compute/Visualize.h"
#include "compute/ColourMap.h"

namespace Speckle {

void Visualize::foreground(ComputePos & pos, double t) {
	cv::Vec3b & pixel = m_out.at<cv::Vec3b>(pos.outY, pos.outX);
	int index = cv::saturate_cast<uint8_t>(256. / t);
	const uint8_t * rgb = ColourMap::plasma[index];
	pixel = (pixel * (1 - m_alpha)) +
		m_alpha * cv::Vec3b(rgb[2], rgb[1], rgb[0]);
}

} // namespace
