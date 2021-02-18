.PHONY = build run clean

TARGET_EXEC := BGP_simulation
CMAKE_BUILD_DIR := cmake-build-debug
BUILD_DIR := $(CMAKE_BUILD_DIR)/build

# Generate the build files
$(CMAKE_BUILD_DIR)/Makefile: CMakeLists.txt
	@echo;
	@echo "[.] Generating build files...";
	cmake -S . -B $(CMAKE_BUILD_DIR)

build: $(CMAKE_BUILD_DIR)/Makefile
	@echo;
	@echo "[.] Building program...";
	@cmake --build $(CMAKE_BUILD_DIR)

run: build
	@echo;
	@echo "[.] Running program...";
	@$(BUILD_DIR)/$(TARGET_EXEC)

clean:
	@echo
	@echo "[.] Cleaning up...";
	rm -rf $(BUILD_DIR);
	rm -rf $(CMAKE_BUILD_DIR);
