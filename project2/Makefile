# Simple Makefile for project2
CXX = g++
CXXFLAGS = -std=c++20 -Wall -O2
OUT = out
TARGET = main
SOURCES = main.cpp Mesh.cpp
LIBS = -lglfw -lGLEW -lGL -lEGL -lm

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