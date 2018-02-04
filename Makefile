
SOURCES = citymap.cpp camera.cpp graphics.cpp imgui.cpp imgui_draw.cpp imgui_impl_glfw_gl3.cpp imgui_demo.cpp appLog.cpp pddltree.cpp agent.cpp
CC = g++
CFLAGS =  -g -I./Include -ltess2 -L. -lGLEW -lGL -lglfw -lSOIL


map: $(SOURCES) map.hpp shaders.glsl ./Include/path_impl.hpp pddltree.hpp
	$(CC) -o map $(SOURCES) $(CFLAGS)


depend: .depend

.depend: $(SOURCES)
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^>>./.depend;

include .depend

gltest: gltest.cpp graphics.cpp
	$(CC) -o gltest gltest.cpp graphics.cpp $(CFLAGS)

.PHONY : clean
clean :
	rm -f map *.o
run:
	optirun ./map ./maps/little.txt
debug:
	optirun 
