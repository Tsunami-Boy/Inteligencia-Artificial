#ifndef UTILS_H
#define UTILS_H

#include <random>

constexpr double EPS = 1e-9;

class Rng {
public:

    explicit Rng(unsigned seed);

    double real();

    int integer(int n);

private:
    std::mt19937 gen_;                              /* motor pseudoaleatorio */
    std::uniform_real_distribution<double> dist_;   /* distribucion [0,1)    */
};

#endif /* UTILS_H */
