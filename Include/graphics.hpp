#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

struct shaderData {
  GLuint vertexShader;
  GLuint fragmentShader;
  GLuint geometryShader;
  GLuint shaderProgram;
  GLuint vbo;
  GLuint vao;
  float* data;
  int vertexCount;
  int gltype;
};

GLuint createShader(GLenum type, const GLchar* src);
shaderData drawLineShaderInit( float* points );
void drawLine(shaderData sh);


#endif
