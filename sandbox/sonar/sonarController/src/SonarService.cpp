#include "SonarService.h"
#include <iostream>

int main()
{
	Matrix m(3,3);
	for (int i = 0 ; i < 9 ; i ++)
		m(i) = i;
	std::cout << m << std::endl;
}

namespace ram {
namespace sonar {

SonarService::SonarService() : m_calibration(4,4) {}

ColumnVector SonarService::localize(const PingRecord_t &rec)
{
	ColumnVector tdoas(4, 1);
	for (octave_idx_type i = 0 ; i < 4 ; i ++)
	{
		Complex c(rec.channels[i].re, rec.channels[i].im);
		tdoas(i) = arg(c);
	}
	ColumnVector kvector = m_calibration * tdoas;
	kvector.resize_no_fill(3);
	return kvector;
}

} // namespace sonar
} // namespace ram
