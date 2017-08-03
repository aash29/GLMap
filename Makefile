map: ex_orig.cpp
	g++ -o map ex_orig.cpp ./Include/nanosvg.c -I./Include -ltess2 -L. -lGLEW -lGL -lglfw
