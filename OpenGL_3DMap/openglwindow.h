#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QTime>
#include <QKeyEvent>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLShader>
#include <QDebug>
#include <QtMath>
#include "resourcemanager.h"

class Camera;

class OpenGLWindow : public QOpenGLWidget
{
public:
    explicit OpenGLWindow(QWidget *parent = nullptr, QString filename=":/res/datas/default.osm");
    ~OpenGLWindow() override;

    void KeyPressEvent(QKeyEvent *event);  //键盘按下事件
    void KeyReleaseEvent(QKeyEvent *event);  //键盘释放事件
    void OpenMapFile(QString path);  //打开地图文件
    void Recovery();  //复原相机

protected:
    void mouseMoveEvent(QMouseEvent *event) override;  //鼠标移动事件
    void mousePressEvent(QMouseEvent *event) override;  //鼠标按下事件
    void mouseReleaseEvent(QMouseEvent *event) override;  //鼠标释放事件
    void wheelEvent(QWheelEvent *event) override;  //滚轮事件

    virtual void resizeGL(int w, int h) override;
    virtual void initializeGL() override;
    virtual void paintGL() override;

private:
    void updateGL();  //更新函数
    void processInput(GLfloat dt);  //摄像机键盘处理函数

    GLboolean keys[1024];
    GLboolean isOpenLighting;
    GLboolean isLineMode;

    GLboolean isFirstMouse;
    GLboolean isLeftMousePress;
    GLint lastX;
    GLint lastY;

    QTime time;
    GLfloat deltaTime;
    GLfloat lastFrame;

    QString m_filename;
    Camera *camera;
    QOpenGLFunctions_3_3_Core *core;

};

/***********相机类**************/
enum Camera_Movement
{
    CAMERA_FORWARD,
    CAMERA_BACKWARD,
    CAMERA_LEFT,
    CAMERA_RIGHT,
    CAMERA_UP,
    CAMERA_DOWN
};

const GLfloat SPEED = 0.25;//速度
const GLfloat SENSITIVITY = 0.1f;//鼠标敏感度
const GLfloat ZOOM = 45.0f;//放大倍数

class Camera
{
public:
    Camera(QVector3D position = QVector3D(0.0f, 0.0f, 0.0f), QVector3D up = QVector3D(0.0f, 1.0f, 0.0f),
           GLfloat yaw = 90.0f, GLfloat pitch = 0.0f): front(QVector3D(0.0f, 0.0f, -1.0f)),
        movementSpeed(SPEED), mouseSensitivity(SENSITIVITY), zoom(ZOOM)
    {
        this->position = position;
        this->worldUp = up;
        this->yaw = yaw;
        this->picth = pitch;
        this->updateCameraVectors();
    }

    QMatrix4x4 getViewMatrix();  //返回lookat函数
    void processMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constraintPitch = true);
    void processMouseScroll(GLfloat yoffset);  //鼠标滚轮事件
    void processKeyboard(Camera_Movement direction, GLfloat deltaTime);  //键盘处理事件函数
    void recoverCamera();

public:
    QVector3D position;  //相机位置
    QVector3D worldUp;
    QVector3D front;

    QVector3D up;  //view坐标系的上方向
    QVector3D right;  //view坐标系的右方向

    GLfloat picth;
    GLfloat yaw;

    GLfloat movementSpeed;
    GLfloat mouseSensitivity;
    GLfloat zoom;

private:
    void updateCameraVectors();
};

/************灯光类**************/
class Light
{
public:
    Light();
    ~Light();
    void init();
    void draw();
private:
    QOpenGLFunctions_3_3_Core *core;
    GLuint VBO;
};

/************坐标类**************/
class Coordinate
{
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
class Image
{
public:
    Image();
    ~Image();
    void init();
    void draw();
private:
    QOpenGLFunctions_3_3_Core *core;
    GLuint positionVBO;
    GLuint VBO;
};

#endif // OPENGLWINDOW_H
