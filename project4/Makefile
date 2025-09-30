# Simple Makefile for project2-new
CXX = g++
CXXFLAGS = -std=c++20 -Wall -g -O0 -Iinclude -I../cyCodebase -I../lodepng
TARGET = main
SOURCES = src/main.cpp ../lodepng/lodepng.cpp
LIBS = -lglfw -lGLEW -lGL -lEGL -lm
OUT = out
ARGS ?= 

# Build target (just compile, don't run)
$(TARGET): $(SOURCES)
	mkdir -p $(OUT)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(OUT)/$(TARGET) $(LIBS)

# Build and run immediately
build-run: $(TARGET)
	./$(OUT)/$(TARGET)

# Clean
clean:
	rm -rf $(OUT)

# Run with optional arguments
run: $(TARGET)
	./$(OUT)/$(TARGET) $(ARGS)

# Pow example
pow_example: pow_example.cpp
	mkdir -p $(OUT)
	$(CXX) $(CXXFLAGS) pow_example.cpp -o $(OUT)/pow_example
	./$(OUT)/pow_example

# TriMesh example
trimesh_example: trimesh_example.cpp
	mkdir -p $(OUT)
	$(CXX) $(CXXFLAGS) trimesh_example.cpp -o $(OUT)/trimesh_example
	./$(OUT)/trimesh_example

.PHONY: clean run pow_example trimesh_example 