map: ex_orig.cpp
	g++ -o map -g ex_orig.cpp ./Include/nanosvg.c -I./Include -ltess2 -L. -lGLEW -lGL -lglfw
.PHONY : clean
clean :
	rm -f map *.o
