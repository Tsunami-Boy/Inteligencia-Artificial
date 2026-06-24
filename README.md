# MS-CFLP-CI

Implementación en C++ de una metaheurística de **Simulated Annealing (SA)** para el problema
*Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities*
(MS-CFLP-CI): se decide qué bodegas abrir y cómo fragmentar la demanda de cada cliente entre
ellas, minimizando costos fijos y de transporte, respetando capacidades e incompatibilidades.

El SA usa representación ad-hoc (matriz de flujos), inicialización aleatoria, un movimiento de
transferencia de flujo ergódico (alcanza todo el espacio de búsqueda) y un esquema de
enfriamiento geométrico con recalentamiento (*reheating*).

## Estructura

```
src/instance.cpp    Carga de la instancia
src/solution.cpp    Representación, evaluación y movimientos de la solución
src/sa.cpp          Simulated Annealing (enfriamiento + recalentamiento)
src/utils.cpp       Generación de números aleatorios
src/main.cpp        Punto de entrada
scripts/run_config.sh   Corre una configuración sobre todas las instancias -> resultados/<cfg>.dat
scripts/comparar.sh     Cruza los .dat y genera estadisticas.txt
instances/          Instancias de prueba (inst01..inst06)
Makefile            Compilación y experimentos
```

## Comandos

En Windows el ejecutable de make es `mingw32-make`; en Linux es `make`.

**Compilar y correr las pruebas** (configuraciones base y agresiva sobre las 6 instancias).
Genera `estadisticas.txt`:

```
# Windows
mingw32-make todo

# Linux
make todo
```

**Compilar y obtener el `out.txt`** (una corrida sobre `instances/inst01.txt`):

```
# Windows
mingw32-make run

# Linux
make run
```

## Ejecución directa del binario

```
./mscflpci <archivo_instancia> [semilla] [-v] [--alpha=A] [--reheats=N]
```

- `semilla` (opcional): entero para reproducibilidad (`0` o ausente = aleatoria).
- `-v` (opcional): muestra el progreso del enfriamiento.
- `--alpha=A` (opcional): factor de enfriamiento geométrico.
- `--reheats=N` (opcional): número de recalentamientos.

Ejemplo:

```
./mscflpci instances/inst01.txt 12345
```

## Formato de instancia

Índices de clientes e instalaciones comienzan en `0`:

```
n m                    (nº de clientes y de instalaciones)
d[0] ... d[n-1]        (demandas)
s[0] ... s[m-1]        (capacidades)
f[0] ... f[m-1]        (costos fijos de apertura)
c[i][0] ... c[i][m-1]  (n filas: costo unitario de transporte)
num_inc                (nº de pares incompatibles)
i k                    (un par incompatible por línea)
```
