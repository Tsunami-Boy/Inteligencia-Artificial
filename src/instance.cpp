#include "instance.h"

#include <fstream>
#include <stdexcept>

Instance Instance::load(const std::string &path)
{
    std::ifstream in(path);
    if (!in)
        throw std::runtime_error("no se pudo abrir el archivo '" + path + "'");

    Instance inst;

    /* --- Dimensiones: primero bodegas (m), luego clientes (n) --- */
    if (!(in >> inst.m)) throw std::runtime_error("error leyendo cantidad de bodegas");
    if (!(in >> inst.n)) throw std::runtime_error("error leyendo cantidad de clientes");
    if (inst.n <= 0 || inst.m <= 0)
        throw std::runtime_error("dimensiones (m n) invalidas");

    const int n = inst.n, m = inst.m;

    inst.d.resize(n);
    inst.s.resize(m);
    inst.f.resize(m);
    inst.c.resize(static_cast<size_t>(n) * m);
    inst.incmat.assign(static_cast<size_t>(n) * n, 0);

    /* --- Capacidades, costos de apertura y demandas --- */
    for (int j = 0; j < m; ++j)
        if (!(in >> inst.s[j])) throw std::runtime_error("error leyendo capacidades");
    for (int j = 0; j < m; ++j)
        if (!(in >> inst.f[j])) throw std::runtime_error("error leyendo costos de apertura");
    for (int i = 0; i < n; ++i)
        if (!(in >> inst.d[i])) throw std::runtime_error("error leyendo demandas");

    /* --- Matriz de costos de transporte (n filas x m columnas) --- */
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            if (!(in >> inst.c[static_cast<size_t>(i) * m + j]))
                throw std::runtime_error("error leyendo costos de transporte");

    /* --- Pares de incompatibilidad --- */
    int num_inc;
    if (!(in >> num_inc) || num_inc < 0)
        throw std::runtime_error("error leyendo numero de incompatibilidades");

    inst.inc.reserve(num_inc);
    for (int p = 0; p < num_inc; ++p) {
        int a, b;
        if (!(in >> a >> b) || a < 0 || a >= n || b < 0 || b >= n)
            throw std::runtime_error("par de incompatibilidad invalido");
        inst.inc.emplace_back(a, b);
        /* Matriz simetrica para consultas en O(1). */
        inst.incmat[static_cast<size_t>(a) * n + b] = 1;
        inst.incmat[static_cast<size_t>(b) * n + a] = 1;
    }

    return inst;
}
