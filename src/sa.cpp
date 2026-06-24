#include "sa.h"
#include "utils.h"

#include <algorithm>
#include <cmath>
#include <iostream>

SAConfig SAConfig::defaults(const Instance &inst)
{
    /* Estimacion de la magnitud de los costos para escalar los parametros. */
    double max_c = 0.0, max_d = 0.0, max_f = 0.0;
    for (int i = 0; i < inst.n; ++i) {
        max_d = std::max(max_d, inst.d[i]);
        for (int j = 0; j < inst.m; ++j)
            max_c = std::max(max_c, inst.cost(i, j));
    }
    for (int j = 0; j < inst.m; ++j)
        max_f = std::max(max_f, inst.f[j]);

    SAConfig cfg;
    cfg.t_initial       = std::max(1.0, max_c * max_d + max_f);  /* ~ peor cambio */
    cfg.t_final         = 1e-3;
    cfg.alpha           = 0.97;
    cfg.iter_per_temp   = 100 * inst.n;
    cfg.max_reheats     = 3;
    cfg.reheat_fraction = 0.5;
    cfg.stagnation_limit = 40;

    /* Penalizaciones grandes para que nunca convenga violar una restriccion. */
    cfg.pen.cap = (max_c + 1.0) * 10.0;
    cfg.pen.inc = (max_c * max_d + max_f + 1.0) * 10.0;

    cfg.seed    = 0;
    cfg.verbose = false;
    return cfg;
}

static bool pick_transfer(const Solution &sol, Rng &rng,
                          int &pi, int &pa, int &pb, double &pqty)
{
    const Instance &inst = sol.instance();
    const int n = inst.n, m = inst.m;
    if (m < 2) return false;

    const int i = rng.integer(n);

    /* Instalacion origen: una de las que abastecen a i (muestreo de reservorio). */
    int a = -1, count = 0;
    for (int j = 0; j < m; ++j) {
        if (sol.flowAt(i, j) > EPS) {
            ++count;
            if (rng.integer(count) == 0) a = j;
        }
    }
    if (a < 0) return false;

    /* Instalacion destino distinta del origen (puede estar cerrada => abrir). */
    int b = rng.integer(m - 1);
    if (b >= a) ++b;

    /* Cantidad: con prob. 1/2 todo el flujo de i en 'a', si no una fraccion. */
    const double flow_a = sol.flowAt(i, a);
    const double qty = (rng.real() < 0.5) ? flow_a : flow_a * rng.real();
    if (qty < EPS) return false;

    pi = i; pa = a; pb = b; pqty = qty;
    return true;
}

Solution sa_run(const Instance &inst, const SAConfig &cfg, long &iterations)
{
    Rng rng(cfg.seed);
    iterations = 0;

    Solution cur(inst);
    cur.randomInit(cfg.pen, rng);   /* inicializacion aleatoria */

    Solution best = cur;
    bool   have_feasible = cur.feasible();
    double best_cost     = cur.cost();
    double best_energy   = cur.energy();

    /* Registra 'cur' como mejor solucion si corresponde: se prefiere una
     * factible de menor costo; si aun no hay factible, la de menor energia. */
    auto consider_best = [&]() {
        if (cur.feasible()) {
            if (!have_feasible || cur.cost() < best_cost) {
                best = cur; best_cost = cur.cost(); have_feasible = true;
            }
        } else if (!have_feasible && cur.energy() < best_energy) {
            best = cur; best_energy = cur.energy();
        }
    };

    /* Fase 0: enfriamiento inicial. Fases 1..max_reheats: recalentamientos. */
    for (int phase = 0; phase <= cfg.max_reheats; ++phase) {
        double T = (phase == 0) ? cfg.t_initial
                                : cfg.t_initial * cfg.reheat_fraction;

        if (cfg.verbose && phase > 0)
            std::cerr << "[SA] --- recalentamiento " << phase << " (T=" << T << ") ---\n";

        int stagnation = 0;
        long level = 0;

        /* Descenso de temperatura geometrico dentro de la fase. */
        while (T > cfg.t_final) {
            const double ref = have_feasible ? best_cost : best_energy;

            for (int it = 0; it < cfg.iter_per_temp; ++it) {
                int i, a, b;
                double qty;
                if (!pick_transfer(cur, rng, i, a, b, qty)) continue;
                ++iterations;  /* un movimiento efectivamente evaluado */

                const double delta = cur.applyTransfer(i, a, b, qty, cfg.pen);

                /* Criterio de aceptacion de Metropolis. */
                const bool accept = (delta <= 0.0) || (rng.real() < std::exp(-delta / T));
                if (!accept) {
                    cur.applyTransfer(i, b, a, qty, cfg.pen);  /* revierte */
                    continue;
                }
                consider_best();
            }

            const double now = have_feasible ? best_cost : best_energy;
            const bool improved = now < ref - EPS;
            stagnation = improved ? 0 : stagnation + 1;

            if (cfg.verbose)
                std::cerr << "[SA] fase=" << phase << " nivel=" << level
                          << " T=" << T << " energia=" << cur.energy()
                          << " mejor_costo=" << best_cost
                          << (have_feasible ? "" : " (sin factible)") << "\n";

            T *= cfg.alpha;  /* descenso de temperatura */
            ++level;

            /* Estancamiento: corta la fase para pasar al recalentamiento. */
            if (stagnation >= cfg.stagnation_limit) break;
        }
    }

    return best;
}