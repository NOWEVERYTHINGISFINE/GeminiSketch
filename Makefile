CXX = g++
CFLAGS = -lpthread -static-libstdc++ -std=c++11

all: main

main: main.o Gemini_elimination.o Gemini_without_switch.o GeminiSketch_Algorithm.o
	$(CXX) -o main main.o Gemini_elimination.o Gemini_without_switch.o GeminiSketch_Algorithm.o $(CFLAGS)

main.o: main.cpp Gemini  elimination.h Gemini without switch.h GeminiSketch_Algorithm.h
	$(CXX) -o main.o -c main.cpp

Gemini_elimination.o: Gemini  elimination.cpp Gemini  elimination.h
	$(CXX) -o Gemini_elimination.o -c Gemini  elimination.cpp

Gemini_without_switch.o: Gemini without switch.cpp Gemini without switch.h
	$(CXX) -o Gemini_without_switch.o -c Gemini without switch.cpp

GeminiSketch_Algorithm.o: GeminiSketch_Algorithm.cpp GeminiSketch_Algorithm.h
	$(CXX) -o GeminiSketch_Algorithm.o -c GeminiSketch_Algorithm.cpp

.PHONY: clean
clean:
	-$(RM) main main.o Gemini_elimination.o Gemini_without_switch.o GeminiSketch_Algorithm.o