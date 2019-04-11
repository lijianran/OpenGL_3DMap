#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>
#include <QMatrix4x4>
#include <QKeyEvent>
#include <QOpenGLShader>//为了glfloat 这些变量，所以引进这个头文件
#include <QDebug>
#include <QtMath>


enum Camera_Movement {
    CAMERA_FORWARD,
    CAMERA_BACKWARD,
    CAMERA_LEFT,
    CAMERA_RIGHT,
    CAMERA_UP,
    CAMERA_DOWN
};

//使用这些宏，可以很方便的修改初始化数据
const GLfloat YAW = 90.0f;//绕y轴旋转
const GLfloat PITCH = 0.0f;//绕x轴旋转 俯仰角
const GLfloat SPEED = 0.25;//速度
const GLfloat SENSITIVITY = 0.1f;//敏感度
const GLfloat ZOOM = 45.0f;

class Camera {
public:
    Camera(QVector3D position = QVector3D(0.0f, 0.0f, 0.0f), QVector3D up = QVector3D(0.0f, 1.0f, 0.0f),
           GLfloat yaw = YAW, GLfloat pitch = PITCH): front(QVector3D(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED),
        mouseSensitivity(SENSITIVITY), zoom(ZOOM){

        this->position = position;
        this->worldUp = up;
        this->yaw = yaw;
        this->picth = pitch;
        this->updateCameraVectors();
    }

    QMatrix4x4 getViewMatrix(); //返回lookat函数
    void processMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constraintPitch = true); //相应鼠标操作函数
    void processMouseScroll(GLfloat yoffset);//鼠标滚轮事件
    void processKeyboard(Camera_Movement direction, GLfloat deltaTime);//键盘处理事件函数
    //void processInput(GLfloat dt);//键盘循环函数

public:
    QVector3D position;
    QVector3D worldUp;
    QVector3D front;

    QVector3D up; //view坐标系 的上方向
    QVector3D right; // view坐标系的 右方向

    //Eular Angles
    GLfloat picth;
    GLfloat yaw;

    //Camera options
    GLfloat movementSpeed;
    GLfloat mouseSensitivity;
    GLfloat zoom;

private:
    void updateCameraVectors();

};
#endif // CAMERA_H
