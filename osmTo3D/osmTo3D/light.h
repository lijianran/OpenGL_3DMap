#ifndef LIGHT_H
#define LIGHT_H

#include <QOpenGLFunctions_3_3_Core>

class Light{
public:
  Light();
  ~Light();
  void init();
  void drawLight();
private:
  QOpenGLFunctions_3_3_Core *core;
  GLuint lightVBO;
};

#endif // LIGHT_H

/************坐标类，一个简单的显示xyz坐标的类**************/
class Coordinate{
public:
  Coordinate();
  ~Coordinate();
  void init();
  void draw();
private:
  QOpenGLFunctions_3_3_Core *core;
  GLuint VBO;
};

/************ 2D纹理映射类，一个简单的显示2D纹理的类**************/
class Image{
public:
  Image();
  ~Image();
  void init();
  void draw();
private:
  QOpenGLFunctions_3_3_Core *core;
  GLuint positionVBO;
  GLuint uvVBO;
};
