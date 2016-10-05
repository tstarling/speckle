#ifndef SPECKLE_VISUALIZE_H
#define SPECKLE_VISUALIZE_H

#include "common/OpenCvTypes.h"
#include "compute/ComputePos.h"

namespace Speckle {

class Visualize {
public:
	Visualize(double minX)
		: m_minX(minX)
	{}

	cv::Vec3b compute(ComputePos & pos, double x);
private:
	double m_minX;
};

} // namespace
#endif
