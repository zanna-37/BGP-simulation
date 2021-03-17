.PHONY = build run clean

TARGET_EXEC := BGP_simulation
BUILD_DIR := cmake-build-debug

# Generate the build files
$(BUILD_DIR)/Makefile: CMakeLists.txt
	@echo;
	@echo "[.] Generating build files...";
	cmake -DTARGET_EXEC:STRING=$(TARGET_EXEC) -S . -B $(BUILD_DIR)

build: $(BUILD_DIR)/Makefile
	@echo;
	@echo "[.] Building program...";
	@cmake --build $(BUILD_DIR)

run: build
	@echo;
	@echo "[.] Running program...";
	@$(BUILD_DIR)/$(TARGET_EXEC)

clean:
	@echo
	@echo "[.] Cleaning up...";
	rm -rf $(BUILD_DIR);
