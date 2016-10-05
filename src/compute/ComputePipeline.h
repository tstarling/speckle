#ifndef SPECKLE_COMPUTEPIPELINE_H
#define SPECKLE_COMPUTEPIPELINE_H

#include "compute/ComputePos.h"
#include "compute/Unpack.h"
#include "compute/SpatialWindow.h"
#include "compute/CorrelationTime.h"
#include "compute/Visualize.h"
#include "common/OpenCvTypes.h"

namespace Speckle {

class ComputePipeline {
public:
	struct Options {
		Options()
			: width(0), height(0), bitsPerPixel(0),
			spatialWindow(7),
			correlationTableSize(1024),
			beta(1.0),
			frameSize(0),
			minX(30)
		{}
			
		int width;
		int height;
		int bitsPerPixel;
		int spatialWindow;
		int correlationTableSize;
		double beta;
		size_t frameSize;
		double minX;
	};

	ComputePipeline(const Options & options);

	void writeFrame(void *data, size_t length, cv::Mat & output, int format);
private:
	Options m_options;

	Unpack m_unpack;
	SpatialWindow m_spatialWindow;
	CorrelationTime m_correlationTime;
	Visualize m_visualize;
};

} // namespace


#endif
