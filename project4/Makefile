# Simple Makefile for project2-new
CXX = g++
CXXFLAGS = -std=c++20 -Wall -O2 -Iinclude -I../cyCodebase
TARGET = main
SOURCES = src/main.cpp
LIBS = -lglfw -lGLEW -lGL -lEGL -lm
OUT = out

# Build target
$(TARGET): $(SOURCES)
	mkdir -p $(OUT)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(OUT)/$(TARGET) $(LIBS)
	./$(OUT)/$(TARGET)

# Clean
clean:
	rm -rf $(OUT)

# Run
run: $(TARGET)
	./$(OUT)/$(TARGET)

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