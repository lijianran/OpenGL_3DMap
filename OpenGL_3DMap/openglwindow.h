#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include <QOpenGLWidget>
#include <QKeyEvent>
#include <QOpenGLFunctions_3_3_Core>
#include <QTime>
#include "camera.h"
#include "resourcemanager.h"


class OpenGLWindow : public QOpenGLWidget
{
public:
    explicit OpenGLWindow(QWidget *parent = nullptr);
    ~OpenGLWindow() override;
    void KeyPressEvent(QKeyEvent *event);  //键盘按下事件
    void KeyReleaseEvent(QKeyEvent *event);  //键盘释放事件

    GLboolean keys[1024];
//    GLboolean isOpenLighting;
    GLboolean isLineMode;

protected:
    void mouseMoveEvent(QMouseEvent *event) override;  //鼠标移动事件
    void mousePressEvent(QMouseEvent *event) override;  //鼠标按下事件
    void mouseReleaseEvent(QMouseEvent *event) override;  //鼠标释放事件
    void wheelEvent(QWheelEvent *event) override;  //滚轮事件

    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

private:
    void processInput(GLfloat dt);  //摄像机键盘处理函数，
    void updateGL();  //更新函数

    QOpenGLFunctions_3_3_Core *core;

    GLboolean isFirstMouse;
    GLboolean isLeftMousePress;
    GLint lastX;
    GLint lastY;

    QTime time;
    GLfloat deltaTime;
    GLfloat lastFrame;//上一帧

    Camera *camera;

};

#endif // OPENGLWINDOW_H

/************灯光类**************/
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

/************坐标类**************/
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

/************2D纹理映射类**************/
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
