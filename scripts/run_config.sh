set -euo pipefail

config="$1"; alpha="$2"; reheats="$3"; seed="$4"; bin="$5"; shift 5

mkdir -p resultados
out="resultados/${config}.dat"
: > "$out"

echo ">> Configuracion '${config}': alpha=${alpha}, reheats=${reheats}, semilla=${seed}"
for inst in "$@"; do
    name=$(basename "$inst" .txt)
    printf '   %-8s ... ' "$name"

    stdout=$("$bin" "$inst" "$seed" --alpha="$alpha" --reheats="$reheats")
    n=$(printf '%s\n' "$stdout" | grep -oE 'n=[0-9]+' | head -1 | cut -d= -f2)
    m=$(printf '%s\n' "$stdout" | grep -oE 'm=[0-9]+' | head -1 | cut -d= -f2)

    read -r Z ms s iter < out.txt

    printf '%s %s %s %s %s %s %s %s\n' \
        "$name" "$n" "$m" "$alpha" "$reheats" "$Z" "$iter" "$ms" >> "$out"
    printf 'Z=%s  iter=%s  t=%sms\n' "$Z" "$iter" "$ms"
done

echo ">> Resultados guardados en ${out}"