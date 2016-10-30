#include "compute/Visualize.h"
#include "compute/ColourMap.h"

namespace Speckle {

cv::Vec3b Visualize::compute(ComputePos & pos, double x) {
	int index = cv::saturate_cast<uint8_t>(256. * m_minX / x);
	const uint8_t * rgb = ColourMap::plasma[index];
	return cv::Vec3b(rgb[2], rgb[1], rgb[0]);
}

} // namespace
