.PHONY = build run clean dbuild drun

TARGET_EXEC := BGP_simulation
BUILD_DIR := cmake-release
DEBUG_DIR := cmake-debug

# Generate the build files
$(BUILD_DIR)/Makefile: CMakeLists.txt
	@echo;
	@echo "[.] Generating build files...";
	cmake -DTARGET_EXEC:STRING=$(TARGET_EXEC) -S . -B $(BUILD_DIR)

$(DEBUG_DIR)/Makefile: CMakeLists.txt
	@echo;
	@echo "[.] Generating debug build files...";
	cmake -DTARGET_EXEC:STRING=$(TARGET_EXEC) -S . -B $(DEBUG_DIR)

build: $(BUILD_DIR)/Makefile
	@echo;
	@echo "[.] Building program...";
	@cmake --build $(BUILD_DIR)

dbuild: $(DEBUG_DIR)/Makefile
	@echo;
	@echo "[.] Building program in DEBUG mode...";
	@cmake -DUSE_DEBUG=ON $(DEBUG_DIR)
	@cmake --build $(DEBUG_DIR)

run: build
	@echo;
	@echo "[.] Running program...";
	@$(BUILD_DIR)/$(TARGET_EXEC)

drun: dbuild
	@echo;
	@echo "[.] Running program in DEBUG mode...";
	@$(DEBUG_DIR)/$(TARGET_EXEC)

clean:
	@echo
	@echo "[.] Cleaning up...";
	rm -rf $(BUILD_DIR);
	rm -rf $(DEBUG_DIR);
