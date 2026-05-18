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
double NumGenerator::exponential(double lambda) {
    double u = (double)rand() / (RAND_MAX + 1.0);
    if (u >= 1.0) u = 1.0 - 1e-12;
    return -std::log(1.0 - u) / lambda;
}
}

