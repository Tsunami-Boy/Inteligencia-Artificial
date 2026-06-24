#include "instance.h"
#include "sa.h"
#include "solution.h"

#include <chrono>
#include <cmath>
#include <exception>
#include <fstream>
#include <iostream>
#include <random>
#include <string>

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <archivo_instancia> [semilla] [-v]\n";
        return 1;
    }

    const std::string path = argv[1];
    unsigned seed = 0;
    bool verbose = false;
    double alpha_override   = -1.0;  /* <0  => usar valor por defecto */
    int    reheats_override = -1;    /* <0  => usar valor por defecto */

    for (int k = 2; k < argc; ++k) {
        const std::string arg = argv[k];
        if      (arg == "-v")                       verbose = true;
        else if (arg.rfind("--alpha=", 0) == 0)     alpha_override   = std::stod(arg.substr(8));
        else if (arg.rfind("--reheats=", 0) == 0)   reheats_override = std::stoi(arg.substr(10));
        else                                        seed = static_cast<unsigned>(std::stoul(arg));
    }

    /* Resuelve la semilla a un valor concreto y reproducible: si el usuario no
     * la fijo (0), se sortea una semilla no nula para poder reportarla. */
    if (seed == 0) {
        std::random_device rd;
        do { seed = rd(); } while (seed == 0);
    }

    try {
        const Instance inst = Instance::load(path);
        std::cout << "Instancia: " << path << "  (n=" << inst.n << " clientes, m="
                  << inst.m << " instalaciones, " << inst.numInc()
                  << " incompatibilidades)\n";

        SAConfig cfg = SAConfig::defaults(inst);
        cfg.seed    = seed;
        cfg.verbose = verbose;
        if (alpha_override > 0.0)    cfg.alpha       = alpha_override;
        if (reheats_override >= 0)   cfg.max_reheats = reheats_override;

        long iterations = 0;
        const auto t0 = std::chrono::steady_clock::now();
        const Solution best = sa_run(inst, cfg, iterations);
        const auto t1 = std::chrono::steady_clock::now();
        const long long elapsed_ms =
            std::llround(std::chrono::duration<double, std::milli>(t1 - t0).count());

        std::cout << "\n===== Mejor solucion encontrada =====\n";
        best.print(std::cout);
        std::cout << "Tiempo de computo: " << elapsed_ms << " ms\n";

        /* Salida a out.txt (valores enteros, salvo la semilla):
         *   linea 1: Z  tiempo(ms)  semilla  iteraciones
         *   lineas 2..n+1: matriz clientes x bodegas con la cantidad satisfecha. */
        const std::string outPath = "out.txt";
        std::ofstream out(outPath);
        if (!out)
            throw std::runtime_error("no se pudo abrir el archivo de salida " + outPath);

        out << static_cast<long long>(std::llround(best.cost())) << " "
            << elapsed_ms << " " << seed << " " << iterations << "\n";
        best.printFlowMatrix(out);

        std::cout << "Resultados escritos en " << outPath << "\n";
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
