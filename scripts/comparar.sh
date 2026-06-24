set -euo pipefail

base="resultados/base.dat"
agr="resultados/agresivo.dat"
out="estadisticas.txt"

have_base=0; have_agr=0
[ -s "$base" ] && have_base=1
[ -s "$agr" ]  && have_agr=1

if [ "$have_base" -eq 0 ] && [ "$have_agr" -eq 0 ]; then
    echo "No hay resultados. Ejecuta 'make base' y/o 'make agresivo' primero." >&2
    exit 1
fi

{
    echo "==================================================================="
    echo " MS-CFLP-CI  -  Recocido Simulado: comparativa de configuraciones"
    echo " Generado: $(date '+%Y-%m-%d %H:%M:%S')   |   Semilla fija: 12345"
    echo "==================================================================="
    echo
    echo " Configuraciones:"
    echo "   base      ->  alpha = 0.97 , recalentamientos = 3"
    echo "   agresivo  ->  alpha = 0.90 , recalentamientos = 5"
    echo

    # --- Resumen individual por configuracion -------------------------------
    for cfg in base agresivo; do
        f="resultados/${cfg}.dat"
        [ -s "$f" ] || continue
        echo "-------------------------------------------------------------------"
        echo " Configuracion: ${cfg}"
        echo "-------------------------------------------------------------------"
        printf '%-8s %4s %4s %7s %8s %12s %10s\n' \
            "Inst" "n" "m" "alpha" "reheats" "Iteraciones" "t(ms)"
        awk '{ printf "%-8s %4s %4s %7s %8s %12s %10s\n", $1,$2,$3,$4,$5,$7,$8 }' "$f"
        echo
    done

    # --- Tabla comparativa (solo si estan ambas) ----------------------------
    if [ "$have_base" -eq 1 ] && [ "$have_agr" -eq 1 ]; then
        echo "==================================================================="
        echo " COMPARATIVA  (base  vs  agresivo)"
        echo "==================================================================="
        printf '%-8s %6s %12s %12s %9s %9s %8s\n' \
            "Inst" "Z" "Iter.base" "Iter.agr" "t.base" "t.agr" "dt%"
        # Une por nombre de instancia y calcula la variacion relativa de tiempo.
        join -j 1 \
            <(awk '{print $1, $6, $7, $8}' "$base" | sort) \
            <(awk '{print $1, $6, $7, $8}' "$agr"  | sort) \
        | awk '{
            name=$1; Zb=$2; itb=$3; tb=$4; Za=$5; ita=$6; ta=$7;
            # dt% = reduccion relativa del tiempo respecto de la base (+ = mas rapido)
            if (tb+0 > 0) dt = (tb-ta)/tb*100; else dt = 0;
            zflag = (Zb==Za) ? "" : "  (!) Z difiere: base="Zb" agr="Za;
            printf "%-8s %6s %12s %12s %9s %9s %+7.1f%%%s\n", name, Zb, itb, ita, tb, ta, dt, zflag;
        }'
        echo
        echo " dt%  = reduccion relativa del tiempo de la config. agresiva"
        echo "        respecto de la base ( + = la agresiva fue mas rapida )."
        echo " Z    = costo objetivo de la mejor solucion (debe coincidir entre"
        echo "        ambas configuraciones bajo la misma semilla)."
    else
        echo "-------------------------------------------------------------------"
        echo " NOTA: falta ejecutar la configuracion '$([ $have_base -eq 0 ] && echo base || echo agresivo)'"
        echo "       para generar la tabla comparativa completa."
        echo "       Ejecuta:  make $([ $have_base -eq 0 ] && echo base || echo agresivo)"
        echo "-------------------------------------------------------------------"
    fi
} > "$out"

echo ">> Informe comparativo escrito en ${out}"
cat "$out"