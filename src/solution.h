#ifndef SOLUTION_H
#define SOLUTION_H

#include <iosfwd>
#include <vector>

#include "instance.h"
#include "utils.h"

/* Pesos de penalizacion para construir la energia a partir del costo real. */
struct Penalties {
    double cap = 0.0;  /* peso por unidad de capacidad excedida          */
    double inc = 0.0;  /* peso por cada par incompatible que comparte j  */
};

class Solution {
public:

    explicit Solution(const Instance &inst);

    void randomInit(const Penalties &pen, Rng &rng);

    void evaluate(const Penalties &pen);

    double applyTransfer(int i, int a, int b, double qty, const Penalties &pen);

    bool feasible() const;

    void print(std::ostream &os) const;

    void printFlowMatrix(std::ostream &os) const;

    /* --- Accesores --- */
    double cost() const { return cost_; }
    double energy() const { return energy_; }
    const Instance &instance() const { return *inst_; }
    double flowAt(int i, int j) const { return flow_[static_cast<size_t>(i) * inst_->m + j]; }
    double usedAt(int j) const { return used_[j]; }

private:
    /* Costo fijo aportado por la instalacion j (f_j si esta en uso, si no 0). */
    double nodeFixed(int j) const;
    /* Numero de pares incompatibles que comparten la instalacion j. */
    int nodeConflicts(int j) const;
    /* Penalizacion de la instalacion j (capacidad excedida + incompatibilidades). */
    double nodePenalty(int j, const Penalties &pen) const;

    const Instance *inst_;
    std::vector<double> flow_;  /* flow_[i*m+j]: cantidad de j a i  (tam. n*m) */
    std::vector<double> used_;  /* used_[j]: carga total de j       (tam. m)   */
    std::vector<char>   open_;  /* open_[j]: 1 si used_[j] > 0      (tam. m)   */
    double cost_ = 0.0;         /* costo objetivo real Z                       */
    double energy_ = 0.0;       /* cost_ + penalizaciones (minimizado por SA)  */
};

#endif /* SOLUTION_H */
