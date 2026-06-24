#include "utils.h"
#include <chrono>

/* Implementacion del generador aleatorio encapsulado en la clase Rng. */

Rng::Rng(unsigned seed)
    : gen_(seed != 0
               ? seed
               : static_cast<unsigned>(
                     std::chrono::steady_clock::now().time_since_epoch().count())),
      dist_(0.0, 1.0)
{
}

double Rng::real()
{
    return dist_(gen_);
}

int Rng::integer(int n)
{
    return static_cast<int>(dist_(gen_) * n);
}
