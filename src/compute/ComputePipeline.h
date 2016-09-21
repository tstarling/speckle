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
			baselineCorrelationTime(1.0),
			frameSize(0),
			alpha(1.0)
		{}
			
		int width;
		int height;
		int bitsPerPixel;
		int spatialWindow;
		int correlationTableSize;
		double baselineCorrelationTime;
		size_t frameSize;
		double alpha;
	};

	ComputePipeline(const Options & options, Mat3b & output);

	void writeFrame(void *data, size_t length);
private:
	Options m_options;

	Unpack m_unpack;
	SpatialWindow m_spatialWindow;
	CorrelationTime m_correlationTime;
	Visualize m_visualize;
};

} // namespace


#endif
