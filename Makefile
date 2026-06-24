
# En Windows, mingw32-make usa cmd.exe por defecto y no entiende los comandos
# de los recipes (bash, rm, ./). Forzamos bash SOLO en Windows; en Linux make
# usa /bin/sh por defecto y no hay que modificar nada.
ifeq ($(OS),Windows_NT)
    SHELL := C:/Program Files/Git/usr/bin/bash.exe
endif

CXX      = g++
CXXFLAGS = -O2 -Wall -Wextra -std=c++17
SRC      = src/main.cpp src/instance.cpp src/solution.cpp src/sa.cpp src/utils.cpp
BIN      = mscflpci

# --- Parametros de los experimentos ----------------------------------------
SEED       = 12345
INSTANCES  = instances/inst01.txt instances/inst02.txt instances/inst03.txt \
             instances/inst04.txt instances/inst05.txt instances/inst06.txt

# Configuracion base (linea base).
ALPHA_BASE   = 0.97
REHEATS_BASE = 3

# Configuracion agresiva (enfriamiento rapido + mas recalentamientos).
ALPHA_AGR    = 0.90
REHEATS_AGR  = 5

# --- Compilacion ------------------------------------------------------------
$(BIN): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(SRC)

# --- Experimento 1: configuracion base --------------------------------------
base: $(BIN)
	@bash scripts/run_config.sh base $(ALPHA_BASE) $(REHEATS_BASE) $(SEED) ./$(BIN) $(INSTANCES)
	@bash scripts/comparar.sh

# --- Experimento 2: enfriamiento agresivo -----------------------------------
agresivo: $(BIN)
	@bash scripts/run_config.sh agresivo $(ALPHA_AGR) $(REHEATS_AGR) $(SEED) ./$(BIN) $(INSTANCES)
	@bash scripts/comparar.sh

# --- Solo (re)generar el informe comparativo a partir de los .dat existentes -
estadisticas:
	@bash scripts/comparar.sh

# --- Ejecutar ambas configuraciones de una sola vez -------------------------
# Tras generar estadisticas.txt, elimina el binario y la carpeta resultados.
todo: base agresivo
	@rm -f $(BIN) $(BIN).exe out.txt
	@rm -rf resultados
	@echo ">> Limpieza: eliminados $(BIN).exe, out.txt y la carpeta resultados/ (se conserva estadisticas.txt)"

# Compila y ejecuta sobre la instancia de ejemplo.
run: $(BIN)
	./$(BIN) instances/inst01.txt $(SEED)

clean:
	rm -f $(BIN) $(BIN).exe out.txt estadisticas.txt
	rm -rf resultados

.PHONY: base agresivo estadisticas todo run clean
