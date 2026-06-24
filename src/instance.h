#ifndef INSTANCE_H
#define INSTANCE_H

#include <string>
#include <utility>
#include <vector>

class Instance {
public:
    int n = 0;  /* numero de clientes (|N|)      */
    int m = 0;  /* numero de instalaciones (|M|) */

    std::vector<double> d;  /* d[i]: demanda del cliente i               */
    std::vector<double> s;  /* s[j]: capacidad de la instalacion j       */
    std::vector<double> f;  /* f[j]: costo fijo de la instalacion j      */
    std::vector<double> c;  /* c[i*m+j]: costo unitario de transporte    */

    std::vector<std::pair<int, int>> inc; /* pares de clientes incompatibles */
    std::vector<char> incmat;             /* incmat[i*n+k]=1 si <i,k> in I   */

    /* cost: costo unitario de transporte c_ij. */
    double cost(int i, int j) const { return c[static_cast<size_t>(i) * m + j]; }

    /* incompatible: true si los clientes i y k no pueden compartir instalacion. */
    bool incompatible(int i, int k) const { return incmat[static_cast<size_t>(i) * n + k] != 0; }

    /* numInc: cantidad de pares incompatibles (|I|). */
    int numInc() const { return static_cast<int>(inc.size()); }

    static Instance load(const std::string &path);
};

#endif /* INSTANCE_H */
