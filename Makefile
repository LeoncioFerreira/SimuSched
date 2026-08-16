CC = gcc
PYTHON ?= python3
CFLAGS = -Wall -Wextra -pthread -g -DUNITY_INCLUDE_DOUBLE -Iinclude -Itests/vendor -Isrc
LDFLAGS = -pthread

# Diretórios
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
TEST_DIR = tests
BIN_DIR = bin

# Arquivos
SOURCES = $(shell find $(SRC_DIR) -name '*.c')
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))
# Objetos para os testes (exclui o main.o do simulador)
TEST_OBJECTS = $(filter-out $(OBJ_DIR)/main.o, $(OBJECTS))

TEST_SOURCES = $(wildcard $(TEST_DIR)/*.test.c)
TEST_BINS = $(patsubst $(TEST_DIR)/%.test.c, $(BIN_DIR)/%, $(TEST_SOURCES))

# Bibliotecas de terceiros (Unity)
UNITY_SRC = $(TEST_DIR)/vendor/unity.c

# Alvo principal (Simulador)
TARGET = $(BIN_DIR)/simulador

# Alvos de execução (Padrão: make run <alvo>)
# Este bloco permite usar 'make run test', 'make run lint', etc.
ifeq (run,$(firstword $(MAKECMDGOALS)))
  RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  $(foreach arg,$(RUN_ARGS),$(if $(filter $(arg),$(shell grep -oE '^[a-zA-Z0-9_-]+:' Makefile | sed 's/://')),,$(eval $(arg):;@:)))
endif

run: $(TARGET)
	@if [ "$(RUN_ARGS)" = "test" ]; then \
		$(MAKE) --no-print-directory test; \
	elif [ "$(RUN_ARGS)" = "lint" ]; then \
		$(MAKE) --no-print-directory lint; \
	elif [ "$(RUN_ARGS)" = "format" ]; then \
		$(MAKE) --no-print-directory format; \
	elif [ -z "$(RUN_ARGS)" ]; then \
		./$(TARGET); \
	else \
		echo "Uso: make run [test|lint|format] ou apenas make run para o simulador"; \
	fi

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para compilar e rodar todos os testes automaticamente
test: test-stats $(TEST_BINS)
	@set -e; for test in $(TEST_BINS); do \
		echo "Executando $$test..."; \
		./$$test; \
	done

test-stats:
	$(PYTHON) -m unittest discover -s tests -p 'test_consolidate_*.py' -v

stats:
	$(PYTHON) scripts/consolidate_statistics.py \
		--input data/raw/simulations.csv \
		--seeds configs/seeds.txt \
		--output-dir data/processed

run-all: $(TARGET)
	@set -eu; \
	seeds_file="configs/seeds.txt"; \
	output_dir="data/raw"; \
	result_file="$$output_dir/simulations.csv"; \
	temp_file="$$output_dir/.simulation.tmp.csv"; \
	algorithms="fcfs priority round-robin tej"; \
	scenarios="balanced cpu_bound io_bound unbalanced_priorities"; \
	[ -f "$$seeds_file" ] || { echo "Erro: arquivo de sementes não encontrado: $$seeds_file" >&2; exit 1; }; \
	awk 'NF { if (NF != 1 || $$1 !~ /^[0-9]+$$/) exit 1; count++ } END { exit count == 100 ? 0 : 1 }' "$$seeds_file" || \
		{ echo "Erro: o arquivo deve conter exatamente 100 sementes numéricas" >&2; exit 1; }; \
	unique_seeds=$$(awk 'NF { print $$1 }' "$$seeds_file" | sort -u | wc -l); \
	[ "$$unique_seeds" -eq 100 ] || { echo "Erro: as 100 sementes devem ser distintas" >&2; exit 1; }; \
	for scenario in $$scenarios; do \
		[ -f "configs/$$scenario.conf" ] || { echo "Erro: cenário não encontrado: configs/$$scenario.conf" >&2; exit 1; }; \
	done; \
	mkdir -p "$$output_dir"; \
	find "$$output_dir" -mindepth 1 -maxdepth 1 ! -name '.gitkeep' -exec rm -rf -- {} +; \
	trap 'rm -f "$$temp_file"' EXIT HUP INT TERM; \
	current=0; total=1600; \
	for scenario in $$scenarios; do \
		for algorithm in $$algorithms; do \
			for seed in $$(awk 'NF { print $$1 }' "$$seeds_file"); do \
				current=$$((current + 1)); \
				echo "[$$current/$$total] cenário=$$scenario algoritmo=$$algorithm seed=$$seed"; \
				./$(TARGET) --algorithm "$$algorithm" --seed "$$seed" \
					--config "configs/$$scenario.conf" --output "$$temp_file" || \
					{ echo "Erro: simulação falhou: cenário=$$scenario algoritmo=$$algorithm seed=$$seed" >&2; exit 1; }; \
				temp_lines=$$(wc -l < "$$temp_file"); \
				[ "$$temp_lines" -eq 2 ] || { echo "Erro: CSV temporário inválido para cenário=$$scenario algoritmo=$$algorithm seed=$$seed" >&2; exit 1; }; \
				if [ "$$current" -eq 1 ]; then \
					mv "$$temp_file" "$$result_file"; \
				else \
					tail -n +2 "$$temp_file" >> "$$result_file"; \
					rm -f "$$temp_file"; \
				fi; \
			done; \
		done; \
	done; \
	line_count=$$(wc -l < "$$result_file"); \
	[ "$$line_count" -eq 1601 ] || { echo "Erro: esperadas 1601 linhas; encontradas $$line_count" >&2; exit 1; }; \
	csv_count=$$(find "$$output_dir" -type f -name '*.csv' | wc -l); \
	[ "$$csv_count" -eq 1 ] || { echo "Erro: esperado um único CSV; encontrados $$csv_count" >&2; exit 1; }; \
	echo "Campanha concluída: $$total resultados consolidados em $$result_file"

# Compilação de cada arquivo de teste
$(BIN_DIR)/%: $(TEST_DIR)/%.test.c $(TEST_OBJECTS) $(UNITY_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

lint:
	cppcheck --enable=all --suppress=missingIncludeSystem -Iinclude src/ include/

format:
	clang-format -i $(SOURCES) $(shell find $(SRC_DIR) -name '*.h') include/*.h $(TEST_SOURCES)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all test test-stats stats lint format clean run run-all
