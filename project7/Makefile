# Makefile for project7 - Rectangle in XY Plane
CXX = g++
CXXFLAGS = -std=c++20 -Wall -O2 -I../cyCodebase
TARGET = main
SOURCES = src/main.cpp
LIBS = -lglfw -lGLEW -lGL -lEGL -lm
OUT = out

# Build target
$(TARGET): $(SOURCES)
	mkdir -p $(OUT)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(OUT)/$(TARGET) $(LIBS)

# Clean
clean:
	rm -rf $(OUT)

# Run
run: $(TARGET)
	./$(OUT)/$(TARGET)

.PHONY: clean run

