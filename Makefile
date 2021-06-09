.PHONY = build clean

# Make all recipes begin with the > character rather than a tab.
.RECIPEPREFIX := >

TARGET_EXEC := BGP_simulation
BUILD_DIR   := cmake-release
DEBUG_DIR   := cmake-debug

# You can set these variables from the command line, and also from the environment.
DEBUG ?= # Put 1 to enable
IWYU  ?= # Put 1 to enable

# Use Clang
export CC                 := clang
export CXX                := clang++
export CMAKE_C_COMPILER   := clang
export CMAKE_CXX_COMPILER := clang++


# Generate the build files
$(BUILD_DIR)/Makefile: CMakeLists.txt
>   @echo;
>   @echo "[.] Generating build files...";
>   cmake -DTARGET_EXEC:STRING=$(TARGET_EXEC) -S . -B $(BUILD_DIR)

$(DEBUG_DIR)/Makefile: CMakeLists.txt
>   @echo;
>   @echo "[.] Generating debug build files...";
>   cmake -DTARGET_EXEC:STRING=$(TARGET_EXEC) -S . -B $(DEBUG_DIR)


# Compile
ifeq ($(DEBUG), 1)
	CHOOSEN_DIR := $(DEBUG_DIR)
else
	CHOOSEN_DIR := $(BUILD_DIR)
endif
build: $(CHOOSEN_DIR)/Makefile
>   @echo;
>   @echo "[.] Building program";
    ifeq ($(IWYU), 1)
>       @echo "[.]   Using Include What You Use (IWYU)...";
>       @cmake -DCMAKE_CXX_INCLUDE_WHAT_YOU_USE="iwyu;-Xiwyu;--no_fwd_decls" $(CHOOSEN_DIR)
    endif

    ifeq ($(DEBUG), 1)
>       @echo "[.]   Enabling DEBUG mode...";
>       @cmake -DUSE_DEBUG=ON $(CHOOSEN_DIR)
>       @cmake -DCMAKE_BUILD_TYPE=Debug $(CHOOSEN_DIR)
    endif
>       @cmake --build $(CHOOSEN_DIR)


# Cleanup
clean:
>   @echo
>   @echo "[.] Cleaning up...";
>   rm -rf $(BUILD_DIR);
>   rm -rf $(DEBUG_DIR);
