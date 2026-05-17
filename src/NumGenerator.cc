//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "NumGenerator.h"
#include <cmath>
#include <cstdlib>

#include "NumGenerator.h"
#include <cmath>
#include <cstdlib>

namespace flora {

NumGenerator::NumGenerator() { }
NumGenerator::~NumGenerator() { }
NumGenerator::NumGenerator(const NumGenerator &other) { }

double NumGenerator::exponential(double lambda) { // 1/lambda = frequency or arrival rate or service rate
    double u = (double)rand() / (RAND_MAX + 1.0);

    if (u >= 1.0) u = 1.0 - 1e-12;

    return -std::log(1.0 - u) / lambda;
}

}

