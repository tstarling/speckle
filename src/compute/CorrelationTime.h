#ifndef SPECKLE_CORRELATIONTIME_H
#define SPECKLE_CORRELATIONTIME_H

#include <cmath>
#include <vector>

#include "compute/ComputePos.h"

namespace Speckle {

class CorrelationTime {
public:
	CorrelationTime(int tableSize, double baseline);
	double compute(ComputePos & pos, double kSq);

private:
	double m_baseline;
	double m_step;
	std::vector<float> m_table;

	static const double tablePrecision;
	static const double asymptoticThreshold;
};

inline double getKSquared(double x) {
	return (std::expm1(-2 * x) + 2 * x) / (2 * std::pow(x, 2));
}

inline double getKSquaredDeriv(double x) {
	// Used Wolfram Alpha
	// It would be more efficient to factor out the exp(-2*x), but that is
	// inaccurate when x≈0 (i.e. k^2≈1)
	return ((std::exp(-2 * x) + 1) * x + std::expm1(-2 * x)) / std::pow(x, 3);
}

inline double doCorrelationIteration(double kSquared, double x0) {
	return x0 - (getKSquared(x0) - kSquared) / getKSquaredDeriv(x0);
}

} // namespace

#endif
