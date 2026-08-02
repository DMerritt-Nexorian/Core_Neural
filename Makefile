# ====================================================================
# Makefile for CORE_NEURAL Real-Time Decoder
# Compliant with MISRA-C:2012 / ISO C18 Standards
# ====================================================================

CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wpedantic -O3 -std=c18 -Iinclude
LDFLAGS = -lm

SRC_DIR = src
TEST_DIR = tests
OBJ_DIR = obj

# Sources & Objects
SRCS = $(SRC_DIR)/tensor_ops.c \
       $(SRC_DIR)/safety_invariants.c \
       $(SRC_DIR)/core_neural_kernels.c

MAIN = $(SRC_DIR)/main.c

OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Targets
MAIN_BIN = core_neural_decoder
UNIT_BIN = unit_tests
FUZZ_BIN = fuzz_harness
INVR_BIN = invariant_validation

.PHONY: all clean run test fuzz verify lint format help

help:
	@echo "CORE_NEURAL high-assurance build commands:"
	@echo "  make all                 - Build all binaries (decoder, tests, fuzz, invariants)"
	@echo "  make run                 - Build and run the main real-time decoder"
	@echo "  make test                - Build and execute the unit test suite"
	@echo "  make fuzz                - Build and execute the pseudo-random stress harness"
	@echo "  make verify              - Build and execute the contractive stability invariant validator"
	@echo "  make static_analysis     - Run local cppcheck static analysis"
	@echo "  make formal_verification - Run local CBMC bounded model checker"
	@echo "  make clean               - Remove all build artifacts and binaries"

all: $(MAIN_BIN) $(UNIT_BIN) $(FUZZ_BIN) $(INVR_BIN)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(MAIN_BIN): $(MAIN) $(OBJS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(UNIT_BIN): $(TEST_DIR)/unit_tests.c $(OBJS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(FUZZ_BIN): $(TEST_DIR)/fuzz_harness.c $(OBJS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(INVR_BIN): $(TEST_DIR)/invariant_validation.c $(OBJS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

run: $(MAIN_BIN)
	./$(MAIN_BIN)

test: $(UNIT_BIN)
	./$(UNIT_BIN)

fuzz: $(FUZZ_BIN)
	./$(FUZZ_BIN)

verify: $(INVR_BIN)
	./$(INVR_BIN)

static_analysis:
	@echo "[STATIC ANALYSIS] Running cppcheck..."
	cppcheck --enable=all --std=c11 --error-exitcode=1 --suppress=missingIncludeSystem -I include $(SRC_DIR)/*.c $(TEST_DIR)/*.c

formal_verification:
	@echo "[FORMAL VERIFICATION] Running CBMC..."
	cbmc $(SRCS) $(TEST_DIR)/unit_tests.c \
		--pointer-check --bounds-check --div-by-zero-check \
		--signed-overflow-check --unsigned-overflow-check \
		-I include --unwind 10

clean:
	rm -rf $(OBJ_DIR) $(MAIN_BIN) $(UNIT_BIN) $(FUZZ_BIN) $(INVR_BIN)
