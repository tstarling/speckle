#ifndef SPECKLE_COMPUTEPOS_H
#define SPECKLE_COMPUTEPOS_H

namespace Speckle {

struct ComputePos {
	ComputePos()
		: x(0), y(0), outX(0), outY(0)
	{}

	int x;
	int y;
	int outX;
	int outY;
};

} // namespace

#endif
