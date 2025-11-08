# Simple Makefile for project2-new
CXX = g++
CXXFLAGS = -std=c++20 -Wall -O2 -Iinclude -I../cyCodebase -I../lodepng
TARGET = main
SOURCES = src/main.cpp ../lodepng/lodepng.cpp
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