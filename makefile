EXE      := Peperobot_Cpp
CC       := g++
EVALFILE ?= networks/nn-5af11540bbfe.nnue
BUILD_DIR := build

all:
	@mkdir -p $(BUILD_DIR)
	@env -u CC -u CXX cmake -S . -B $(BUILD_DIR) \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_C_COMPILER=gcc \
		-DCMAKE_CXX_COMPILER=$(CC) \
		-DEVALFILE="$(EVALFILE)"
	@cmake --build $(BUILD_DIR) --target Peperobot_Cpp -j
	@if [ -f "$(BUILD_DIR)/Peperobot_Cpp.exe" ]; then \
		cp $(BUILD_DIR)/Peperobot_Cpp.exe $(EXE).exe; \
	else \
		cp $(BUILD_DIR)/Peperobot_Cpp $(EXE); \
	fi