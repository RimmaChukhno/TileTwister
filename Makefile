# Nom
TARGET = main.exe

# Sources
SRC = main.cpp Game.cpp Tile.cpp Window.cpp Renderer.cpp Utils.cpp Score.cpp
TEST_SRC = tests.cpp Game.cpp Utils.cpp

# Compilateur
CXX = g++

# Flags
CXXFLAGS = -std=gnu++17 -Wall -O2 -I/mingw64/include
LDFLAGS = -L/mingw64/lib -lmingw32 -lSDL2main -lSDL2 -mwindows

TEST_CXXFLAGS = $(CXXFLAGS) -DSDL_MAIN_HANDLED

# Règle par défaut
all: $(TARGET)

# Compilation principale
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# Tests
test: tests.exe

tests.exe: $(TEST_SRC)
	$(CXX) $(TEST_CXXFLAGS) -o tests.exe $(TEST_SRC)

# Nettoyage
clean:
	rm -f $(TARGET) tests.exe
