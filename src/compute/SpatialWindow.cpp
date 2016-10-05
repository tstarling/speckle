#include "compute/SpatialWindow.h"
#include <cstdio>

namespace Speckle {

SpatialWindow::SpatialWindow(int window, int width)
	: m_window(window),
	m_width(width),
	m_area(window * window),
	m_top(0),
	m_pivot(0)
{
	for (int y = 0; y < window + 1; y++) {
		m_buffer.emplace_back(width);
	}
}

double SpatialWindow::compute(ComputePos & pos, int value) {
	const int x = pos.x;
	const int y = pos.y;

	if (x == 0 && y > m_window) {
		// Rotate the buffer, drop a row from the top
		m_top++;
		if (m_pivot == m_window) {
			m_pivot = -1;
		}
		m_pivot++;
	}
	PixelStats & current = getBufferEntry(x, y);

	// Record value and value squared
	current.value = value;
	int valueSq = value * value;
	current.valueSq = valueSq;

	// Vertical (subtotal) sums
	current.vertSum = value;
	current.vertSumSq = valueSq;

	if (y > 0) {
		PixelStats & previous = getBufferEntry(x, y - 1);
		current.vertSum += previous.vertSum;
		current.vertSumSq += previous.vertSumSq;
	}

	int window = m_window;
	int halfWindow = (window - 1) / 2;
	bool valid = true;

	if (y >= window) {
		PixelStats & top = getBufferEntry(x, y - window);
		current.vertSum -= top.value;
		current.vertSumSq -= top.valueSq;
	} else {
		valid = false;
	}

	// Horizontal (grand total) sums
	current.horizSum = current.vertSum;
	current.horizSumSq = current.vertSumSq;
	if (x > 0) {
		PixelStats & previous = getBufferEntry(x - 1, y);
		current.horizSum += previous.horizSum;
		current.horizSumSq += previous.horizSumSq;
	}
	if (x >= window) {
		PixelStats & left = getBufferEntry(x - window, y);
		current.horizSum -= left.vertSum;
		current.horizSumSq -= left.vertSumSq;
	} else {
		valid = false;
	}

	if (!valid) {
		return 0.0;
	}

	pos.outX = x - halfWindow;
	pos.outY = y - halfWindow;

	double kSquared;

	if (current.horizSum == 0) {
		kSquared = 0.0;
	} else {
		kSquared =
			(double)(
				(int64_t)m_area * current.horizSumSq - 
				(int64_t)current.horizSum * current.horizSum
			) / (m_area - 1)
			/ current.horizSum / current.horizSum * m_area;
	}

	/*
	static int sample = 0;
	if (sample++ > 100000) {
		sample = 0;
		std::printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%g\n",
				pos.x, pos.y, pos.outX, pos.outY,
				current.value, current.valueSq,
					current.vertSum, current.vertSumSq,
				current.horizSum, current.horizSumSq, kSquared);
	}
	*/

	return kSquared;
}

} // namespace
