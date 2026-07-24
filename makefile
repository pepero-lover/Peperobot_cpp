EXE      := Peperobot_Cpp
CC       ?= g++
EVALFILE ?= networks/nn-5af11540bbfe.nnue
BUILD_DIR := build

.DEFAULT_GOAL := all

all:
	@mkdir -p $(BUILD_DIR)
	@cmake -S . -B $(BUILD_DIR) \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_CXX_COMPILER=$(CC) \
		$(if $(EVALFILE),-DEVALFILE="$(EVALFILE)",)
	@cmake --build $(BUILD_DIR) --target Peperobot_Cpp -j
	@cp $(BUILD_DIR)/Peperobot_Cpp $(EXE)

clean:
	rm -rf $(BUILD_DIR) $(EXE)