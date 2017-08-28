
SOURCES = citymap.cpp camera.cpp
CC = g++
CFLAGS =  -g -I./Include -ltess2 -L. -lGLEW -lGL -lglfw


map: $(SOURCES) map.hpp
	$(CC) -o map $(SOURCES) $(CFLAGS)


depend: .depend

.depend: $(SOURCES)
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^>>./.depend;

include .depend

.PHONY : clean
clean :
	rm -f map *.o
