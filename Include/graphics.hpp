#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP
//#include <GL/gl.h>
#include <GL/glew.h>
//#include "camera.hpp"

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

  GLuint texture;
};

GLuint createShader(GLenum type, const GLchar* src);
shaderData drawLineShaderInit( float* points, int numPoints );
void drawLine(shaderData sh, Camera cam, GLfloat r, GLfloat g, GLfloat b, GLfloat w=0.0015f);
shaderData drawMapShaderInit(const float* verts, const int nverts, const int* elements, const  int nelement );
void drawMap( shaderData sh, Camera cam);
shaderData drawBuildingOutlinesInit(float* verts, const int nverts);
void drawBuildingOutlines( shaderData sh, Camera cam);
shaderData texQuadInit();
void texQuadDraw(shaderData sh, int posx, int posy);
shaderData drawQuadInit();
void drawQuad(shaderData sh, float posx, float posy);

shaderData drawPathGraphShaderInit( float* points, int numPoints);
void drawPathGraph(shaderData sh, Camera cam, GLfloat r, GLfloat g, GLfloat b);

shaderData drawThinLineShaderInit( float* points, int numPoints);
void drawThinLine(shaderData sh, Camera cam, GLfloat r, GLfloat g, GLfloat b);

void freeQuadDraw(shaderData sh, float x, float y);

extern float angleNorth;


#endif
