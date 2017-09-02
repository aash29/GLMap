#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include "camera.hpp"

struct shaderData {
  GLuint vertexShader;
  GLuint fragmentShader;
  GLuint geometryShader;
  GLuint shaderProgram;
  GLuint vbo;
  GLuint vao;
  GLuint ebo;

  float* data;
  int vertexCount;
  int elementCount;
  int gltype;
};

GLuint createShader(GLenum type, const GLchar* src);
shaderData drawLineShaderInit( float* points, int numPoints );
void drawLine(shaderData sh);
shaderData drawMapShaderInit(const float* verts, const int nverts, const int* elements, const  int nelement );
void drawMap( shaderData sh, Camera cam);





#endif
