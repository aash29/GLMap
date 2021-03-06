#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <thread>

#include "graphics.hpp"
#include "camera.hpp"
#include "shaders.glsl"
#include <iostream>

#include <SOIL/SOIL.h>

//float angleNorth= 0.f;

float gridSize = Camera::gridSize;

//float gridSize = 0.05f;

glm::mat4 setupCam()
{
  glm::mat4 proj = g_camera.BuildProjectionMatrix();

  glm::mat4 translate1 = glm::translate(glm::mat4(1.f),glm::vec3(-g_camera.m_center.x,-g_camera.m_center.y,0.f));
  
  glm::mat4 rotN = glm::rotate(glm::mat4(1.f), glm::radians(g_camera.angleNorth), glm::vec3(0.0f, 0.0f, 1.0f));

  glm::mat4 translate2 =  glm::translate(glm::mat4(1.f),glm::vec3(g_camera.m_center.x,g_camera.m_center.y,0.f));
  
  glm::mat4 trans = proj*translate2*rotN*translate1;

  //glm::mat4 trans = Model*rotN;
  return trans;
};

static void sCheckGLError() {
    GLenum errCode = glGetError();
    if (errCode != GL_NO_ERROR) {
        fprintf(stderr, "OpenGL error = %d\n", errCode);
        assert(false);
    }
}


// Prints shader compilation errors
static void sPrintLog(GLuint object) {
  GLint log_length = 0;
  if (glIsShader(object))
    glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
  else if (glIsProgram(object))
    glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
  else {
    fprintf(stderr, "printlog: Not a shader or a program\n");
    return;
  }

  char *log = (char *) malloc(log_length);

  if (glIsShader(object))
    glGetShaderInfoLog(object, log_length, NULL, log);
  else if (glIsProgram(object))
    glGetProgramInfoLog(object, log_length, NULL, log);

  fprintf(stderr, "%s", log);
  free(log);
}


//
static GLuint sCreateShaderFromString(const char *source, GLenum type) {
  GLuint res = glCreateShader(type);
  const char *sources[] = {source};
  glShaderSource(res, 1, sources, NULL);
  glCompileShader(res);
  GLint compile_ok = GL_FALSE;
  glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
  if (compile_ok == GL_FALSE) {
    fprintf(stderr, "Error compiling shader of type %d!\n", type);
    sPrintLog(res);
    glDeleteShader(res);
    return 0;
  }

  return res;
}

//
static GLuint sCreateShaderProgram(const char *vs, const char *fs) {
  GLuint vsId = sCreateShaderFromString(vs, GL_VERTEX_SHADER);
  GLuint fsId = sCreateShaderFromString(fs, GL_FRAGMENT_SHADER);
  assert(vsId != 0 && fsId != 0);

  GLuint programId = glCreateProgram();
  glAttachShader(programId, vsId);
  glAttachShader(programId, fsId);
  glBindFragDataLocation(programId, 0, "color");
  glLinkProgram(programId);

  glDeleteShader(vsId);
  glDeleteShader(fsId);

  GLint status = GL_FALSE;
  glGetProgramiv(programId, GL_LINK_STATUS, &status);
  assert(status != GL_FALSE);

  return programId;
}


GLuint createShader(GLenum type, const GLchar* src) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);

  GLint status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  char buffer[512];
  glGetShaderInfoLog(shader, 512, NULL, buffer);
    
  if (status != GL_TRUE)
    {
      printf("%s\n", buffer);
    }
    
  return shader;
}


shaderData drawPathGraphShaderInit( float* points, int numPoints) {
  shaderData outSh;

  outSh.vertexShader = createShader(GL_VERTEX_SHADER, pathGraphVertexShaderSrc);
  outSh.fragmentShader = createShader(GL_FRAGMENT_SHADER, pathGraphFragmentShaderSrc);
  outSh.geometryShader = createShader(GL_GEOMETRY_SHADER, pathGraphGeometryShaderSrc);

  outSh.shaderProgram = glCreateProgram();



  glAttachShader(outSh.shaderProgram, outSh.vertexShader);
  glAttachShader(outSh.shaderProgram, outSh.fragmentShader);
  glAttachShader(outSh.shaderProgram, outSh.geometryShader);

  glLinkProgram(outSh.shaderProgram);
  glUseProgram(outSh.shaderProgram);


  glGenBuffers(1, &outSh.vbo);

  outSh.data = points;
  outSh.vertexCount = numPoints;


  glBindBuffer(GL_ARRAY_BUFFER, outSh.vbo);
  glBufferData(GL_ARRAY_BUFFER, outSh.vertexCount*2*sizeof(float), outSh.data, GL_STATIC_DRAW);

  // Create VAO
  glGenVertexArrays(1, &outSh.vao);
  glBindVertexArray(outSh.vao);

  // Specify layout of point data
  GLint posAttrib = glGetAttribLocation(outSh.shaderProgram, "pos");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

  return outSh;


}


void drawPathGraph(shaderData sh, Camera cam, GLfloat r, GLfloat g, GLfloat b) {


    glBindVertexArray(sh.vao);

    glUseProgram(sh.shaderProgram);

    const glm::mat4 trans = setupCam();

    GLuint uniTrans = glGetUniformLocation(sh.shaderProgram, "Model");

  glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));

  glm::mat4 transInv = glm::inverse(trans);

  GLuint uniTransInv = glGetUniformLocation(sh.shaderProgram, "ModelInv");

  glUniformMatrix4fv(uniTransInv, 1, GL_FALSE, glm::value_ptr(transInv));



    GLint uniColor = glGetUniformLocation(sh.shaderProgram, "lineColor");

    glUniform4f(uniColor, r,g,b,1.f);

    glDrawArrays(GL_LINES, 0, sh.vertexCount);

}


shaderData drawLineShaderInit( float* points, int numPoints) {

  shaderData outSh;
  
  outSh.vertexShader = createShader(GL_VERTEX_SHADER, lineVertexShaderSrc);
  outSh.fragmentShader = createShader(GL_FRAGMENT_SHADER, lineFragmentShaderSrc);
  outSh.geometryShader = createShader(GL_GEOMETRY_SHADER, lineGeometryShaderSrc);

  outSh.shaderProgram = glCreateProgram();

  

  glAttachShader(outSh.shaderProgram, outSh.vertexShader);
  glAttachShader(outSh.shaderProgram, outSh.fragmentShader);
  glAttachShader(outSh.shaderProgram, outSh.geometryShader);
  
  glLinkProgram(outSh.shaderProgram);
  glUseProgram(outSh.shaderProgram);

  
  glGenBuffers(1, &outSh.vbo);

  outSh.data = points;
  outSh.vertexCount = numPoints;
  

  glBindBuffer(GL_ARRAY_BUFFER, outSh.vbo);
  glBufferData(GL_ARRAY_BUFFER, outSh.vertexCount*2*sizeof(float), outSh.data, GL_DYNAMIC_DRAW);

  // Create VAO
  glGenVertexArrays(1, &outSh.vao);
  glBindVertexArray(outSh.vao);

  // Specify layout of point data
  GLint posAttrib = glGetAttribLocation(outSh.shaderProgram, "pos");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

  return outSh;
  
}

void drawLine(shaderData sh, Camera cam, GLfloat r, GLfloat g, GLfloat b, GLfloat w) {


  glBindVertexArray(sh.vao);
  
  glUseProgram(sh.shaderProgram);
  /*
    glm::mat4 Model = cam.BuildProjectionMatrix();

    glm::mat4 rotN;

    rotN = glm::rotate(rotN, glm::radians(angleNorth), glm::vec3(0.0f, 0.0f, 1.0f));
  */
  glm::mat4 trans = setupCam();
  
  GLuint uniTrans = glGetUniformLocation(sh.shaderProgram, "Model");
  
  glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));

  GLint uniColor = glGetUniformLocation(sh.shaderProgram, "lineColor");
  
  glUniform4f(uniColor, r,g,b,1.f);

  GLint linewidth = glGetUniformLocation(sh.shaderProgram, "linewidth");

  glUniform1f(linewidth, w);
   
  glDrawArrays(GL_LINES, 0, sh.vertexCount);

};


shaderData drawThinLineShaderInit( float* points, int numPoints) {

  shaderData outSh;

    outSh.vertexShader = createShader(GL_VERTEX_SHADER, vertexSource);
    outSh.fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentSource);

    assert(outSh.vertexShader != 0 && outSh.fragmentShader != 0);

    outSh.shaderProgram = glCreateProgram();



    glAttachShader(outSh.shaderProgram, outSh.vertexShader);
    glAttachShader(outSh.shaderProgram, outSh.fragmentShader);
    //glAttachShader(outSh.shaderProgram, outSh.geometryShader);

    glLinkProgram(outSh.shaderProgram);

    //sCheckGLError();

    GLint status = GL_FALSE;
    glGetProgramiv(outSh.shaderProgram, GL_LINK_STATUS, &status);
    assert(status != GL_FALSE);


    glUseProgram(outSh.shaderProgram);



    glGenBuffers(1, &outSh.vbo);

    outSh.data = points;
    outSh.vertexCount = numPoints;


    glBindBuffer(GL_ARRAY_BUFFER, outSh.vbo);
    glBufferData(GL_ARRAY_BUFFER, outSh.vertexCount*2*sizeof(float), outSh.data, GL_DYNAMIC_DRAW);

    // Create VAO
    glGenVertexArrays(1, &outSh.vao);
    glBindVertexArray(outSh.vao);

    // Specify layout of point data
    //GLint posAttrib = glGetAttribLocation(outSh.shaderProgram, "position");
    GLint m_vertexAttribute = 0;
    glEnableVertexAttribArray(m_vertexAttribute);
    glVertexAttribPointer(m_vertexAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);

    //sCheckGLError();

    return outSh;

}

void drawThinLine(shaderData sh, Camera cam, GLfloat r, GLfloat g, GLfloat b) {


    glBindVertexArray(sh.vao);

    glUseProgram(sh.shaderProgram);

    glm::mat4 trans = setupCam();

    GLuint uniTrans = glGetUniformLocation(sh.shaderProgram, "Model");

    glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));

    GLint uniColor = glGetUniformLocation(sh.shaderProgram, "setColor");

    glUniform4f(uniColor, r,g,b,1.f);

    glDrawArrays(GL_LINES, 0, sh.vertexCount);

};


shaderData drawBuildingOutlinesInit(float* verts, const int nverts)
{
  shaderData outSh;

  outSh.vertexShader = createShader(GL_VERTEX_SHADER, vertexSource);
  outSh.fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentSource);
 
  outSh.shaderProgram = glCreateProgram(); 

  glAttachShader(outSh.shaderProgram, outSh.vertexShader);
  glAttachShader(outSh.shaderProgram, outSh.fragmentShader);
 
  //glBindFragDataLocation(outSh.shaderProgram, 0, "outColor");
  
  glLinkProgram(outSh.shaderProgram);
  glUseProgram(outSh.shaderProgram);

  glGenBuffers(1, &outSh.vbo); // Generate 1 buffer

  outSh.data = verts;
  outSh.vertexCount = nverts;

  
  glBindBuffer(GL_ARRAY_BUFFER, outSh.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*nverts*2, verts, GL_STATIC_DRAW);

  
  glGenVertexArrays(1, &outSh.vao);
  
  glBindVertexArray(outSh.vao);
  
  

  GLint posAttrib = glGetAttribLocation(outSh.shaderProgram, "position");
  glEnableVertexAttribArray(posAttrib);			
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
			
  return outSh;
  
};

void drawBuildingOutlines( shaderData sh, Camera cam)
{
  glBindVertexArray(sh.vao);
  
  glUseProgram(sh.shaderProgram);
  /*
    glm::mat4 Model = cam.BuildProjectionMatrix();

    glm::mat4 rotN;

    rotN = glm::rotate(rotN, glm::radians(angleNorth), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 trans = Model*rotN;
  */
  glm::mat4 trans = setupCam();
  GLuint uniTrans = glGetUniformLocation(sh.shaderProgram, "Model");
  
  glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));


  GLint uniColor = glGetUniformLocation(sh.shaderProgram, "setColor");
  
  glUniform4f(uniColor, 1.0f, 1.0f, 1.0f, 1.0f);
   
  glDrawArrays(GL_LINES, 0, sh.vertexCount);

};

shaderData drawMapShaderInit(const float* verts, const int nverts, const int* elements, const  int nelements )
{
  shaderData outSh;

  outSh.vertexCount = nverts;
  
  outSh.vertexShader = createShader(GL_VERTEX_SHADER, vertexSource);
  outSh.fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentSource);
 
  outSh.shaderProgram = glCreateProgram(); 

  glAttachShader(outSh.shaderProgram, outSh.vertexShader);
  glAttachShader(outSh.shaderProgram, outSh.fragmentShader);
 
  glBindFragDataLocation(outSh.shaderProgram, 0, "outColor");
  
  glLinkProgram(outSh.shaderProgram);
  glUseProgram(outSh.shaderProgram);

  glGenVertexArrays(1, &outSh.vao);
  
  glBindVertexArray(outSh.vao);
  
  
  glGenBuffers(1, &outSh.vbo); // Generate 1 buffer
  
  glBindBuffer(GL_ARRAY_BUFFER, outSh.vbo);
  
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*nverts*2, verts, GL_STATIC_DRAW);

  glGenBuffers(1, &outSh.ebo);
  
  outSh.elementCount = nelements;
  // printf("nelements:%d \n", nelements);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, outSh.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	       sizeof(int)*nelements*3, elements, GL_STATIC_DRAW);

  GLint posAttrib = glGetAttribLocation(outSh.shaderProgram, "position");
			
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
			
  glEnableVertexAttribArray(posAttrib);

  /*
    cam.m_center = glm::vec2(0.5f,0.5f);

    //  float proj[16] = { 0.0f };
    glm::mat4 Model = cam.BuildProjectionMatrix();

    uniTrans = glGetUniformLocation(shaderProgram, "Model");
    glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(Model));
  */
  return outSh;
};

void drawMap( shaderData sh, Camera cam){

  glBindVertexArray(sh.vao);
  
  glUseProgram(sh.shaderProgram);

  glm::mat4 trans = setupCam();
  
  GLuint uniTrans = glGetUniformLocation(sh.shaderProgram, "Model");
  
  glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));

  GLint uniColor = glGetUniformLocation(sh.shaderProgram, "setColor");
    /*
    glEnableVertexAttribArray(colAttrib);
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE,
                          5*sizeof(float), (void*)(2*sizeof(float)));
  */
  glUniform4f(uniColor, 0.2f, 0.4f, 0.2f, 1.0f);

  
  glDrawElements(GL_TRIANGLES, sh.elementCount*3, GL_UNSIGNED_INT, 0);
   
}


shaderData drawQuadInit()
{
  shaderData qShader;
  qShader.vertexCount = 4;
  
  // Create Vertex Array Object
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  qShader.vao = vao;
 
  // Create a Vertex Buffer Object and copy the vertex data to it
  GLuint vbo;
  glGenBuffers(1, &vbo);

  qShader.vbo = vbo;
  
  int posx = 0;

  int posy = 0;

  GLfloat vertices[] = {
    //  Position      Color             Texcoords
    (posx - 0.5f)*gridSize, (posy + 0.5f)*gridSize , 1.0f, 1.0f, 1.0f,  // Top-left
    (posx + 0.5f)*gridSize,  (posy + 0.5f)*gridSize, 1.0f, 1.0f, 1.0f, // Top-right
    (posx + 0.5f)*gridSize, (posy - 0.5f)*gridSize, 1.0f, 1.0f, 1.0f,  // Bottom-right
    (posx - 0.5f)*gridSize, (posy - 0.5f)*gridSize, 1.0f, 1.0f, 1.0f  // Bottom-left
  };

  qShader.data = vertices;
 
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Create an element array
  GLuint ebo;
  glGenBuffers(1, &ebo);

  GLuint elements[] = {
    0, 1, 2,
    2, 3, 0
  };

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

  qShader.vertexShader = createShader(GL_VERTEX_SHADER, vertexSource );

  qShader.fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentSource);
 
  // Link the vertex and fragment shader into a shader program
  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, qShader.vertexShader);
  glAttachShader(shaderProgram, qShader.fragmentShader);
  glBindFragDataLocation(shaderProgram, 0, "outColor");
  glLinkProgram(shaderProgram);
  glUseProgram(shaderProgram);


  qShader.shaderProgram = shaderProgram;

    // Specify the layout of the vertex data

  GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE,
			5*sizeof(float), 0);

  GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
  glEnableVertexAttribArray(colAttrib);
  glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE,
			5*sizeof(float), (void*)(2*sizeof(float)));

  

  return qShader;  
}


void drawQuad(shaderData sh, float posx, float posy)
{
  glBindVertexArray(sh.vao);
  
  glUseProgram(sh.shaderProgram);
  
  glm::mat4 trans = setupCam();
  
  float x = posx* gridSize;
  float y = posy* gridSize;
  
  
  glm::mat4 translate1 = glm::translate(glm::mat4(1.f),glm::vec3(x,y,1.f));

  glm::mat4 m1 = trans*translate1;
  
  GLuint uniTrans = glGetUniformLocation(sh.shaderProgram, "Model");
  
  glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(m1));


  GLint uniColor = glGetUniformLocation(sh.shaderProgram, "setColor");
  
  glUniform4f(uniColor, 1.0f, 1.0f, 1.0f, 0.3f);

  
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
};


shaderData texQuadInit()
{
  shaderData tqShader;

  tqShader.vertexCount = 4;
  
  // Create Vertex Array Object
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  tqShader.vao = vao;
 
  // Create a Vertex Buffer Object and copy the vertex data to it
  GLuint vbo;
  glGenBuffers(1, &vbo);

  tqShader.vbo = vbo;

  float tlx = 0.35f;
  float tly = 0.65f;

  float brx = 0.37f;
  float bry = 0.75f;

  int posx = 0;

  int posy = 0;

  GLfloat vertices[] = {
    //  Position      Color             Texcoords
    posx*gridSize, (posy+1)*gridSize , 1.0f, 1.0f, 1.0f, tlx, tly, // Top-left
    (posx+1)*gridSize,  (posy+1)*gridSize, 1.0f, 1.0f, 1.0f, brx, tly, // Top-right
    (posx+1)*gridSize, (posy)*gridSize, 1.0f, 1.0f, 1.0f, brx, bry, // Bottom-right
    posx*gridSize, (posy)*gridSize, 1.0f, 1.0f, 1.0f, tlx, bry  // Bottom-left
  };

  tqShader.data = vertices;
 
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Create an element array
  GLuint ebo;
  glGenBuffers(1, &ebo);

  GLuint elements[] = {
    0, 1, 2,
    2, 3, 0
  };

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

  tqShader.vertexShader = createShader(GL_VERTEX_SHADER, texQuadVertex );

  tqShader.fragmentShader = createShader(GL_FRAGMENT_SHADER, texQuadFragment);
 
  // Link the vertex and fragment shader into a shader program
  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, tqShader.vertexShader);
  glAttachShader(shaderProgram, tqShader.fragmentShader);
  glBindFragDataLocation(shaderProgram, 0, "outColor");
  glLinkProgram(shaderProgram);
  glUseProgram(shaderProgram);


  tqShader.shaderProgram = shaderProgram; 
  // Specify the layout of the vertex data
  GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);

  GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
  glEnableVertexAttribArray(colAttrib);
  glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

  GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
  glEnableVertexAttribArray(texAttrib);
  glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));

  // Load textures
  GLuint textures[1];
  glGenTextures(1, textures);

  tqShader.texture = textures[0];
  int width, height;
  unsigned char* image;

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textures[0]);
  image = SOIL_load_image(".png", &width, &height, 0, SOIL_LOAD_RGBA);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
  SOIL_free_image_data(image);
  glUniform1i(glGetUniformLocation(shaderProgram, "texKitten"), 0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


  return tqShader;
}


void texQuadDraw(shaderData sh, int posx, int posy)
{
  glBindVertexArray(sh.vao);
  
  glUseProgram(sh.shaderProgram);
  
  glActiveTexture(GL_TEXTURE0);

  glBindTexture(GL_TEXTURE_2D, sh.texture);

  glUniform1i(glGetUniformLocation(sh.shaderProgram, "texKitten"), 0);

  glm::mat4 trans = setupCam();
  
  float x = posx* gridSize;
  float y = posy* gridSize;
  
  
  glm::mat4 translate1 = glm::translate(glm::mat4(1.f),glm::vec3(x,y,0.f));

  glm::mat4 m1 = trans*translate1;
  
  GLuint uniTrans = glGetUniformLocation(sh.shaderProgram, "Model");
  
  glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(m1));
  
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
};



void freeQuadDraw(shaderData sh, float x, float y) {
    glBindVertexArray(sh.vao);

    glUseProgram(sh.shaderProgram);

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, sh.texture);

    glUniform1i(glGetUniformLocation(sh.shaderProgram, "texKitten"), 0);

    glm::mat4 trans = setupCam();

    glm::mat4 translate1 = glm::translate(glm::mat4(1.f), glm::vec3(x, y, 0.f));

    glm::mat4 m1 = trans * translate1;

    GLuint uniTrans = glGetUniformLocation(sh.shaderProgram, "Model");

    glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(m1));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}


/*
struct GLRenderLines {
    void Create() {
      const char *vs = \
        "#version 400\n"
              "uniform mat4 projectionMatrix;\n"
              "layout(location = 0) in vec2 v_position;\n"
              "layout(location = 1) in vec4 v_color;\n"
              "out vec4 f_color;\n"
              "void main(void)\n"
              "{\n"
              "	f_color = v_color;\n"
              "	gl_Position = projectionMatrix * vec4(v_position, 0.0f, 1.0f);\n"
              "}\n";

      const char *fs = \
        "#version 400\n"
              "in vec4 f_color;\n"
              "out vec4 color;\n"
              "void main(void)\n"
              "{\n"
              "	color = f_color;\n"
              "}\n";

      m_programId = sCreateShaderProgram(vs, fs);
      m_projectionUniform = glGetUniformLocation(m_programId, "projectionMatrix");
      m_vertexAttribute = 0;
      m_colorAttribute = 1;

      // Generate
      glGenVertexArrays(1, &m_vaoId);
      glGenBuffers(2, m_vboIds);

      glBindVertexArray(m_vaoId);
      glEnableVertexAttribArray(m_vertexAttribute);
      glEnableVertexAttribArray(m_colorAttribute);

      // Vertex buffer
      glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
      glVertexAttribPointer(m_vertexAttribute, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
      glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_DYNAMIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
      glVertexAttribPointer(m_colorAttribute, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
      glBufferData(GL_ARRAY_BUFFER, sizeof(m_colors), m_colors, GL_DYNAMIC_DRAW);

      sCheckGLError();

      // Cleanup
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      m_count = 0;
    }

    void Destroy() {
      if (m_vaoId) {
        glDeleteVertexArrays(1, &m_vaoId);
        glDeleteBuffers(2, m_vboIds);
        m_vaoId = 0;
      }

      if (m_programId) {
        glDeleteProgram(m_programId);
        m_programId = 0;
      }
    }

    void Vertex(const b2Vec2 &v, const b2Color &c) {
      if (m_count == e_maxVertices)
        Flush();

      m_vertices[m_count] = v;
      m_colors[m_count] = c;
      ++m_count;
    }

    void Flush() {
      if (m_count == 0)
        return;

      glUseProgram(m_programId);

      float32 proj[16] = {0.0f};
      g_camera.BuildProjectionMatrix(proj, 0.1f);

      glUniformMatrix4fv(m_projectionUniform, 1, GL_FALSE, proj);

      glBindVertexArray(m_vaoId);

      glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
      glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(b2Vec2), m_vertices);

      glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
      glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(b2Color), m_colors);

      glDrawArrays(GL_LINES, 0, m_count);

      sCheckGLError();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
      glUseProgram(0);

      m_count = 0;
    }

    enum {
        e_maxVertices = 2 * 512
    };
    b2Vec2 m_vertices[e_maxVertices];
    b2Color m_colors[e_maxVertices];

    int32 m_count;

    GLuint m_vaoId;
    GLuint m_vboIds[2];
    GLuint m_programId;
    GLint m_projectionUniform;
    GLint m_vertexAttribute;
    GLint m_colorAttribute;
};

*/



/*


  const char* circleSource = R"glsl(
  #version 330
    
  in vec2 fPosition;
  out vec4 fColor;

  void main() {
  vec4 colors[4] = vec4[](
  vec4(1.0, 0.0, 0.0, 1.0), 
  vec4(0.0, 1.0, 0.0, 1.0), 
  vec4(0.0, 0.0, 1.0, 1.0), 
  vec4(0.0, 0.0, 0.0, 1.0)
  );
  fColor = vec4(1.0);

  for(int row = 0; row < 2; row++) {
  for(int col = 0; col < 2; col++) {
  float dist = distance(fPosition, vec2(-0.50 + col, 0.50 - row));
  float alpha = step(0.45, dist);
  fColor = mix(colors[row*2+col], fColor, alpha);
  }
  }
  }
  )glsl";


  GLuint circleShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(circleShader, 1, &circleSource, NULL);
  glCompileShader(circleShader);

  GLint status0;
  glGetShaderiv(circleShader, GL_COMPILE_STATUS, &status0);

  char buffer0[512];

  glGetShaderInfoLog(circleShader, 512, NULL, buffer0);

  if (status0 != GL_TRUE)
  {
  printf("%s\n", buffer0);
  }



  const char* circleSource = R"glsl(
  #version 330
    
  in vec2 fPosition;
  out vec4 fColor;

  void main() {
  vec4 colors[4] = vec4[](
  vec4(1.0, 0.0, 0.0, 1.0), 
  vec4(0.0, 1.0, 0.0, 1.0), 
  vec4(0.0, 0.0, 1.0, 1.0), 
  vec4(0.0, 0.0, 0.0, 1.0)
  );
  fColor = vec4(1.0);

  for(int row = 0; row < 2; row++) {
  for(int col = 0; col < 2; col++) {
  float dist = distance(fPosition, vec2(-0.50 + col, 0.50 - row));
  float delta = fwidth(dist);
  float alpha = smoothstep(0.15-delta, 0.15, dist);
  fColor = mix(colors[row*2+col], fColor, alpha);
  }
  }
  }
  )glsl";


  GLuint circleVertexShader = createShader(GL_VERTEX_SHADER,vertexSource);
  GLuint circleShader = createShader(GL_FRAGMENT_SHADER,circleSource);

  GLuint circleProgram = glCreateProgram();
  glAttachShader(circleProgram, circleVertexShader);
  glAttachShader(circleProgram, circleShader);

  //glBindFragDataLocation(shaderProgram, 0, "outColor");

  glLinkProgram(circleProgram);



  GLint circlePosAttrib = glGetAttribLocation(circleProgram, "position");

  glVertexAttribPointer(circlePosAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

  glEnableVertexAttribArray(circlePosAttrib);

  glUseProgram(circleProgram);  circleProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(circleProgram, circleShader);
  
  //glBindFragDataLocation(shaderProgram, 0, "outColor");

  glLinkProgram(circleProgram);





*/
