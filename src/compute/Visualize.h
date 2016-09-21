#ifndef SPECKLE_VISUALIZE_H
#define SPECKLE_VISUALIZE_H

#include "common/OpenCvTypes.h"
#include "compute/ComputePos.h"

namespace Speckle {

class Visualize {
public:
	Visualize(Mat3b & out, int width, int height, double alpha)
		: m_out(out), m_alpha(alpha)
	{
		out.create(height, width);		
	}

	void background(ComputePos & pos, cv::Vec3b color) {
		m_out.at<cv::Vec3b>(pos.y, pos.x) = color;
	}

	void foreground(ComputePos & pos, double t);
private:
	Mat3b & m_out;
	double m_alpha;
};

} // namespace
#endif
