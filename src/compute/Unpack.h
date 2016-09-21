#ifndef SPECKLE_UNPACK_H
#define SPECKLE_UNPACK_H

#include <stdexcept>
#include <cstdint>
#include <limits>

#include "compute/ComputePos.h"

namespace Speckle {

/**
 * Unpack a bit-packed representation of a luminance stream. Output pixels
 * as ints.
 */
class Unpack {
public:
	Unpack(size_t frameSize, int bitsPerPixel)
		: m_frameSize(frameSize), m_bpp(bitsPerPixel),
		m_mask((1 << bitsPerPixel) - 1), m_pos(nullptr), m_end(nullptr),
		m_buffer(0)
	{
		if (bitsPerPixel > std::numeric_limits<int>::digits) {
			throw std::runtime_error("Too many bits per pixel");
		}	
	}

	void startFrame(void *data) {
		m_pos = static_cast<uint8_t*>(data);
		m_end = m_pos + m_frameSize;
		m_buffer = 0;
	}

	int compute(ComputePos & pos) {
		while (m_bufferSize < m_bpp) {
			if (m_pos >= m_end) {
				throw std::runtime_error("Attempted to read beyond the end of the input buffer");
			}

			m_buffer = (m_buffer << 8) | *(m_pos++);
			m_bufferSize += 8;
		}
		m_bufferSize -= m_bpp;
		return (m_buffer >> m_bufferSize) & m_mask;
	}
private:
	size_t m_frameSize;
	int m_bpp;

	int m_mask;
	uint8_t *m_pos;
	uint8_t *m_end;
	unsigned int m_buffer;
	int m_bufferSize;
};

} // namespace

#endif
