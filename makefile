# Compiler
CXX = g++

# Directories
SRC_DIR = src
CIMG_DIR = $(SRC_DIR)/CImg-3.5.4_pre04072515
BIN_DIR = bin

# Flags
CXXFLAGS = -std=c++20 -O2 -Wall -Wextra -I$(SRC_DIR) -I$(CIMG_DIR)
LDFLAGS =
LIBS = -lpthread -lpng -lz

# Files
MAIN_SRC = $(SRC_DIR)/main.cpp
TARGET_NAME = main
TARGET = $(BIN_DIR)/$(TARGET_NAME)
HEADERS = $(CIMG_DIR)/CImg.h
OBJECTS = $(MAIN_SRC:.cpp=.o)

.PHONY: all clean run checklibs

all: $(TARGET)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(TARGET): $(MAIN_SRC) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(MAIN_SRC) -o $(TARGET) $(LDFLAGS) $(LIBS)
	@echo "Build complete. Executable: $(TARGET)"

clean:
	rm -vf $(TARGET)

run: $(TARGET)
	./$(TARGET)

checklibs:
	@echo "Checking for required libraries..."
	@ldconfig -p | grep -q libpng && echo " [OK] libpng found" || echo " [!!] libpng potentially missing (install libpng-dev or equivalent)"
	@ldconfig -p | grep -q libz && echo " [OK] zlib found" || echo " [!!] zlib potentially missing (install zlib1g-dev or equivalent)"
	@echo "Note: This is a basic check. Actual linking depends on system setup."
