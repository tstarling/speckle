#ifndef SPECKLE_SPATIALWINDOW_H
#define SPECKLE_SPATIALWINDOW_H

#include <stdexcept>
#include <vector>
#include "compute/ComputePos.h"

namespace Speckle {

class SpatialWindow {
public:
	SpatialWindow(int window, int width);

	void startFrame() {
		m_top = -1;
		m_pivot = -1;
	}

	double compute(ComputePos & pos, int value);

private:

	struct PixelStats {
		int value;
		int valueSq;
		int vertSum;
		int vertSumSq;
		int horizSum;
		int horizSumSq;
	};

	static_assert(sizeof(PixelStats) < sizeof(int) * 12, "Inefficient structure padding");

	PixelStats & getBufferEntry(int x, int y) {
		int rowIndex = (y - m_top + m_pivot) % (m_window + 1);
		return m_buffer.at(rowIndex).at(x);
	}

	std::vector<std::vector<PixelStats>> m_buffer;
	const int m_window;
	const int m_width;
	const int m_area;
	int m_top;
	int m_pivot;
};

} // namespace

#endif
