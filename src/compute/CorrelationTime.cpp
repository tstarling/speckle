#include <stdexcept>
#include "compute/CorrelationTime.h"

#include <iostream>

namespace Speckle {

// The precision of precomputed values in the lookup table
const double CorrelationTime::tablePrecision = 1e-6;

// The exact point at which the asymptotic approximation is better depends on
// table size, but the error is pretty small below 0.05
const double CorrelationTime::asymptoticThreshold = 0.05;

CorrelationTime::CorrelationTime(int tableSize, double beta)
	: m_beta(beta),
	m_step(1.0 / (tableSize - 1)),
	m_table(tableSize)
{
	m_table[0] = 0.0;

	for (int i = 1; i < tableSize; i++) {
		double kSq = m_step * i;
		double x = 1.0 / kSq;
		int j;
		double relError = 1.;
		double expected;
		for (j = 0; j < 100 && relError > tablePrecision; j++) {
			x = doCorrelationIteration(kSq, x);
			expected = getKSquared(x);
			relError = std::abs(expected - kSq) / expected;
		}
		if (!std::isnormal(relError) || relError > tablePrecision) {
			throw std::runtime_error("Correlation time solution did not converge");
		}
		m_table[i] = (float)x;
	}
}

double CorrelationTime::compute(ComputePos & pos, double kSq) {
	double x;
	kSq /= m_beta;
	if (kSq < m_step || kSq < asymptoticThreshold) {
		// The asymptotic approximation is good when k^2 is small
		x = 1.0 / kSq - 0.5;
	} else if (kSq >= 1.0) {
		// The solution is 0 at k^2 = 1, and negative for k^2>1, which is unphysical
		x = 0.;
	} else {
		// Look up the seed value in the table, then do a single iteration of the
		// Newton method
		x = m_table.at(std::round(kSq / m_step));
		x = doCorrelationIteration(kSq, x);
	}

	return x;
}

} // namespace
