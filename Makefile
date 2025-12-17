# Nom
TARGET = main.exe

# Sources
SRC = main.cpp Game.cpp Tile.cpp Window.cpp Renderer.cpp Utils.cpp
TEST_SRC = tests.cpp Game.cpp Utils.cpp
TEST_CXXFLAGS = $(CXXFLAGS) -DSDL_MAIN_HANDLED

# Compiler and flags
CXX = g++
CXXFLAGS = -I"C:/msys64/mingw64/include" -Wall -O2
LDFLAGS = -L"C:/msys64/mingw64/lib" -lmingw32 -lSDL2main -lSDL2 -mwindows

# Default rule
all: $(TARGET)

# Compile
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

test: tests.exe

tests.exe: $(TEST_SRC)
	$(CXX) $(TEST_CXXFLAGS) -o tests.exe $(TEST_SRC)

# Cleaning
clean:
	cmd /c if exist $(TARGET) del /f /q $(TARGET)
	cmd /c if exist tests.exe del /f /q tests.exe
