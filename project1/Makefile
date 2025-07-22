# Simple Makefile for project1
CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2
TARGET = main
SOURCES = main.cpp
LIBS = -lglfw -lGLEW -lGL -lEGL -lm

# Build target
$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LIBS)

# Clean
clean:
	rm -f $(TARGET)

# Run
run: $(TARGET)
	./$(TARGET)

.PHONY: clean run 