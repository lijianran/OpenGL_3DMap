#ifndef CARCONTROL_H
#define CARCONTROL_H

#include "opensmloading.h"
#include "model.h"
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
  void setCarModel(const QString& path); //设置汽车模型，传参为路径
  void setOSM(OpenSMLoading *osm);
  void setCarOriginMatrix(QVector3D trans, QVector3D scale, GLfloat rotAngle = 0.0f, QVector3D rotAxis = QVector3D(0, 1, 0)); //设置汽车模型的初始参数,rotAngle旋转角度，rotAxis旋转角度
  QMatrix4x4 getModelMatrix();

  void draw(GLboolean isOpenLighting = GL_FALSE);
  void processKeyboard(Car_Movement direction, GLfloat deltaTime);
private:
  OpenSMLoading *osm;

  /*********** 汽车 参数 *************/
  Model *model;              //汽车模型
  QVector3D position;        //汽车位置
  QVector3D front;           //汽车前进方向
  GLfloat movementSpeed;     //汽车前进速度
  GLfloat turnSpeed;         //汽车转弯速度
  GLfloat yaw;               //汽车偏转角

  QMatrix4x4 originModel;    //汽车模型的初始参数控制，矫正初始模型的大小，平移与旋转
};

#endif // CARCONTROL_H
