map: ex_orig.cpp camera.cpp
	g++ -o map -g ex_orig.cpp camera.cpp ./Include/nanosvg.c -I./Include -ltess2 -L. -lGLEW -lGL -lglfw
.PHONY : clean
clean :
	rm -f map *.o
