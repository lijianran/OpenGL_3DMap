#ifndef CARCONTROL_H
#define CARCONTROL_H

#include "loadmapdata.h"
#include "loadmodel.h"
#include <QtMath>

enum Car_Movement{
    CAR_STRAIGHT,
    CAR_BACKWARD,
    CAR_LEFT,
    CAR_RIGHT
};

class CarControl{
public:
    CarControl();
    ~CarControl();
    void setCarModel(const QString& path); //设置车辆模型，传参为路径
    void setOSM(LoadMapData *osm);
    void setCarOriginMatrix(QVector3D trans, QVector3D scale, GLfloat rotAngle = 0.0f, QVector3D rotAxis = QVector3D(0, 1, 0)); //设置车辆模型的初始参数,rotAngle旋转角度，rotAxis旋转角度
    QMatrix4x4 getModelMatrix();

    void draw(GLboolean isOpenLighting = GL_FALSE);
    void processKeyboard(Car_Movement direction, GLfloat deltaTime);
private:
    LoadMapData *osm;

    /*********** 车辆参数 *************/
    Model *model;              //车辆模型
    QVector3D position;        //车辆位置
    QVector3D front;           //车辆前进方向
    GLfloat movementSpeed;     //车辆前进速度
    GLfloat turnSpeed;         //车辆转弯速度
    GLfloat yaw;               //车辆偏转角

    QMatrix4x4 originModel;    //车辆模型的初始参数控制，矫正初始模型的大小，平移与旋转
};

#endif // CARCONTROL_H
