#ifndef SA_H
#define SA_H

#include "instance.h"
#include "solution.h"

/* Parametros de configuracion del algoritmo. */
struct SAConfig {
    double t_initial = 1.0;        /* temperatura inicial                     */
    double t_final   = 1e-3;       /* temperatura de termino de cada fase     */
    double alpha     = 0.97;       /* factor de enfriamiento geometrico       */
    int    iter_per_temp = 1000;   /* iteraciones por nivel de temperatura    */

    int    max_reheats   = 3;      /* numero de recalentamientos              */
    double reheat_fraction = 0.5;  /* fraccion de t_initial al recalentar     */
    int    stagnation_limit = 40;  /* niveles sin mejora que cortan una fase  */

    Penalties pen;                 /* pesos de penalizacion                   */
    unsigned  seed = 0;            /* semilla (0 => reloj)                    */
    bool      verbose = false;     /* imprimir progreso por stderr            */

    static SAConfig defaults(const Instance &inst);
};

Solution sa_run(const Instance &inst, const SAConfig &cfg, long &iterations);

#endif /* SA_H */
