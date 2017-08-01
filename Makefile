map: example.cpp
	g++ -o map example.cpp ./Include/nanosvg.c -I./Include -ltess2 -L. -lGLEW -lGL -lglfw
