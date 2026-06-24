#include "solution.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <ostream>

Solution::Solution(const Instance &inst)
    : inst_(&inst),
      flow_(static_cast<size_t>(inst.n) * inst.m, 0.0),
      used_(inst.m, 0.0),
      open_(inst.m, 0)
{
}

double Solution::nodeFixed(int j) const
{
    return (used_[j] > EPS) ? inst_->f[j] : 0.0;
}

int Solution::nodeConflicts(int j) const
{
    const int n = inst_->n, m = inst_->m;
    int cnt = 0;
    for (int i = 0; i < n; ++i) {
        if (flow_[static_cast<size_t>(i) * m + j] <= EPS) continue;
        for (int k = i + 1; k < n; ++k)
            if (flow_[static_cast<size_t>(k) * m + j] > EPS && inst_->incompatible(i, k))
                ++cnt;
    }
    return cnt;
}

double Solution::nodePenalty(int j, const Penalties &pen) const
{
    double over = used_[j] - inst_->s[j];
    if (over < 0.0) over = 0.0;
    return pen.cap * over + pen.inc * nodeConflicts(j);
}

void Solution::randomInit(const Penalties &pen, Rng &rng)
{
    const int n = inst_->n, m = inst_->m;

    std::fill(flow_.begin(), flow_.end(), 0.0);
    std::fill(used_.begin(), used_.end(), 0.0);

    std::vector<int> idx(m);
    for (int i = 0; i < n; ++i) {
        /* Baraja (Fisher-Yates) las instalaciones con el generador del proyecto. */
        for (int j = 0; j < m; ++j) idx[j] = j;
        for (int j = m - 1; j > 0; --j) std::swap(idx[j], idx[rng.integer(j + 1)]);

        /* Numero aleatorio de fuentes (entre 1 y m/2, redondeo hacia arriba) y pesos aleatorios. */
        const int kmax = (m + 1) / 2;
        const int k = 1 + rng.integer(kmax);
        std::vector<double> w(k);
        double sum = 0.0;
        for (int t = 0; t < k; ++t) { w[t] = rng.real() + 1e-6; sum += w[t]; }

        for (int t = 0; t < k; ++t) {
            const int j = idx[t];
            const double qty = inst_->d[i] * w[t] / sum;
            flow_[static_cast<size_t>(i) * m + j] += qty;
            used_[j] += qty;
        }
    }

    evaluate(pen);
}

void Solution::evaluate(const Penalties &pen)
{
    const int n = inst_->n, m = inst_->m;
    double cost = 0.0, penalty = 0.0;

    /* Costo de transporte: suma de c_ij * flujo. */
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            cost += inst_->cost(i, j) * flow_[static_cast<size_t>(i) * m + j];

    /* Costos fijos y penalizaciones por instalacion. */
    for (int j = 0; j < m; ++j) {
        open_[j] = (used_[j] > EPS) ? 1 : 0;
        cost    += nodeFixed(j);
        penalty += nodePenalty(j, pen);
    }

    cost_   = cost;
    energy_ = cost + penalty;
}

double Solution::applyTransfer(int i, int a, int b, double qty, const Penalties &pen)
{
    const int m = inst_->m;

    /* Aportes ANTES, separando costo fijo (objetivo real) de penalizacion. */
    const double before_fixed = nodeFixed(a) + nodeFixed(b);
    const double before_pen   = nodePenalty(a, pen) + nodePenalty(b, pen);

    /* Aplica el movimiento de flujo. */
    flow_[static_cast<size_t>(i) * m + a] -= qty;
    flow_[static_cast<size_t>(i) * m + b] += qty;
    used_[a] -= qty;
    used_[b] += qty;

    /* Limpieza numerica para no arrastrar residuos negativos diminutos. */
    if (flow_[static_cast<size_t>(i) * m + a] < EPS) flow_[static_cast<size_t>(i) * m + a] = 0.0;
    if (used_[a] < EPS) used_[a] = 0.0;

    open_[a] = (used_[a] > EPS) ? 1 : 0;
    open_[b] = (used_[b] > EPS) ? 1 : 0;

    /* Aportes DESPUES. */
    const double after_fixed = nodeFixed(a) + nodeFixed(b);
    const double after_pen   = nodePenalty(a, pen) + nodePenalty(b, pen);

    /* El costo de transporte es lineal: solo cambia por la cantidad movida. */
    const double dtrans = qty * (inst_->cost(i, b) - inst_->cost(i, a));

    const double dcost = (after_fixed - before_fixed) + dtrans;  /* objetivo real  */
    const double dpen  = (after_pen - before_pen);               /* penalizaciones */

    cost_   += dcost;
    energy_ += dcost + dpen;

    return dcost + dpen;  /* variacion de energia */
}

bool Solution::feasible() const
{
    const int m = inst_->m;
    for (int j = 0; j < m; ++j) {
        if (used_[j] > inst_->s[j] + EPS) return false;  /* capacidad     */
        if (nodeConflicts(j) > 0)         return false;  /* incompatibles */
    }
    return true;
}

void Solution::print(std::ostream &os) const
{
    const int n = inst_->n, m = inst_->m;
    const std::ios::fmtflags flags = os.flags();

    os << std::fixed << std::setprecision(4);
    os << "Costo objetivo Z : " << cost_ << "\n";
    os << "Factible         : " << (feasible() ? "SI" : "NO") << "\n";

    os << std::setprecision(2);
    os << "Instalaciones abiertas:";
    for (int j = 0; j < m; ++j)
        if (open_[j])
            os << " j" << j << "(uso=" << used_[j] << "/" << inst_->s[j] << ")";
    os << "\n";

    os << "Asignaciones por cliente:\n";
    for (int i = 0; i < n; ++i) {
        os << "  cliente " << i << " (d=" << inst_->d[i] << "):";
        for (int j = 0; j < m; ++j) {
            const double fl = flow_[static_cast<size_t>(i) * m + j];
            if (fl > EPS)
                os << " [j" << j << ": " << fl << " ("
                   << std::setprecision(1) << 100.0 * fl / inst_->d[i]
                   << std::setprecision(2) << "%)]";
        }
        os << "\n";
    }

    os.flags(flags);
}

void Solution::printFlowMatrix(std::ostream &os) const
{
    const int n = inst_->n, m = inst_->m;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            if (j) os << ' ';
            os << static_cast<long long>(std::llround(flow_[static_cast<size_t>(i) * m + j]));
        }
        os << "\n";
    }
}
