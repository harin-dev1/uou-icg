# Makefile for project8 - Tessellation
CXX = g++
CXXFLAGS = -std=c++20 -Wall -O2 -I../cyCodebase -I../lodepng
TARGET = main
SOURCES = src/main.cpp ../lodepng/lodepng.cpp
LIBS = -lglfw -lGLEW -lGL -lGLU -lEGL -lm
OUT = out
ARGS ?= 
# Build target
$(TARGET): $(SOURCES)
	mkdir -p $(OUT)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(OUT)/$(TARGET) $(LIBS)

# Clean
clean:
	rm -rf $(OUT)

# Run
run: $(TARGET)
	./$(OUT)/$(TARGET) $(ARGS)

.PHONY: clean run

