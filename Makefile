.PHONY = build run clean

TARGET_EXEC := output
BUILD_DIR := ./build
SRC_DIR := ./src ./include

# Find all the C and C++ files we want to compile
SRCS := $(shell find $(SRC_DIR) -name *.cpp)

# String substitution for every C/C++ file.
# As an example, hello.cpp turns into ./build/hello.cpp.o
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := $(shell find $(SRC_DIR) -type d)
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS := -g -ansi -pedantic -W -Wall -Werror -std=c++11
LIBRARIES := ./lib/libtins.a


# The final build step.
build: $(BUILD_DIR)/$(TARGET_EXEC)

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	@echo;
	@echo "[.] Building executable...";
	$(CXX) $(CXXFLAGS) $(INC_FLAGS) $(CPPFLAGS) $(OBJS) $(LIBRARIES) -o $(BUILD_DIR)/$(TARGET_EXEC)

# Build step for C++ source
$(OBJS): $(SRCS)
	@echo;
	@echo "[.] Building objects...";
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INC_FLAGS) $(CPPFLAGS) -c $< -o $@

run: build
	@echo;
	@echo "[.] Running program...";
	@$(BUILD_DIR)/$(TARGET_EXEC)

clean:
	@echo
	@echo "[.] Cleaning up...";
	rm -rf $(BUILD_DIR);
