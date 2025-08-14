# Simple Makefile for project2-new
CXX = g++
CXXFLAGS = -std=c++20 -Wall -O2 -Iinclude
TARGET = main
SOURCES = src/main.cpp src/Matrix4x4.cpp src/Mesh.cpp
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

.PHONY: clean run 