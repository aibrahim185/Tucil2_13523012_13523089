# Compiler
CXX = g++

# Directories
SRC_DIR = src
CIMG_DIR = $(SRC_DIR)/CImg-3.5.4_pre04072515
BIN_DIR = bin
CXXFLAGS = -std=c++11 -O2 -Wall -Wextra -I$(SRC_DIR) -I$(CIMG_DIR)

LDFLAGS =

LIBS = -lpthread -lpng -lz

MAIN_SRC = $(SRC_DIR)/main.cpp

TARGET_NAME = main

TARGET = $(BIN_DIR)/$(TARGET_NAME)

HEADERS = $(CIMG_DIR)/CImg.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(MAIN_SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(MAIN_SRC) -o $(TARGET) $(LDFLAGS) $(LIBS)
	@echo "Build complete. Executable: $(TARGET)"

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)