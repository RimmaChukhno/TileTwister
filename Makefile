BUILD_DIR = build
TARGET = $(BUILD_DIR)/main.exe
TEST_TARGET = $(BUILD_DIR)/tests.exe

MSYS_BIN = C:/msys64/usr/bin
MKDIR_P = $(MSYS_BIN)/mkdir.exe -p
RM_F = $(MSYS_BIN)/rm.exe -f

SRC = \
	$(wildcard src/app/*.cpp) \
	$(wildcard src/core/*.cpp) \
	$(wildcard src/game/*.cpp) \
	$(wildcard src/platform/*.cpp) \
	$(wildcard src/render/*.cpp)

TEST_SRC = \
	$(wildcard tests/*.cpp) \
	$(wildcard src/core/*.cpp) \
	$(wildcard src/game/*.cpp)

# Compiler and flags
CXX = g++
CXXFLAGS = -Iinclude -I"C:/msys64/mingw64/include" -Wall -O2
LDFLAGS = -L"C:/msys64/mingw64/lib" -lmingw32 -lSDL2main -lSDL2 -mwindows

# Default rule
all: $(TARGET)

$(BUILD_DIR):
	$(MKDIR_P) $(BUILD_DIR)

# Compile
$(TARGET): $(BUILD_DIR) $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

test: $(TEST_TARGET)

$(TEST_TARGET): $(BUILD_DIR) $(TEST_SRC)
	$(CXX) $(CXXFLAGS) -DSDL_MAIN_HANDLED -o $(TEST_TARGET) $(TEST_SRC)

# Cleaning
clean:
	$(RM_F) $(TARGET) $(TEST_TARGET)
