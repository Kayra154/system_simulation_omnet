#ifndef NUMGENERATOR_H_
#define NUMGENERATOR_H_
namespace flora {
class NumGenerator {
public:
    NumGenerator();
    virtual ~NumGenerator();
    NumGenerator(const NumGenerator &other);
    static double exponential(double lambda);
};
}
#endif
