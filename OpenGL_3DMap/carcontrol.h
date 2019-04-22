#ifndef CARCONTROL_H
#define CARCONTROL_H

#include "loadmapdata.h"
#include "resourcemanager.h"
#include <QOpenGLFunctions_3_3_Core>
#include <QVector>
#include <QDebug>
#include <QVector3D>
#include <QVector2D>
#include <QOpenGLTexture>
#include <QMap>
#include <QtMath>
#include <QFile>

class Object;
class Material;
class Model;

enum Car_Movement
{
    CAR_STRAIGHT,
    CAR_BACKWARD,
    CAR_LEFT,
    CAR_RIGHT
};

class CarControl
{
public:
    CarControl();
    ~CarControl();
    void setCarModel(const QString& path); //设置车辆模型，传参为路径
    void setMap(LoadMapData *map);
    void setCarOriginMatrix(QVector3D trans, QVector3D scale,
                            GLfloat rotAngle = 0.0f, QVector3D rotAxis = QVector3D(0, 1, 0));
                            //设置车辆模型的初始参数,rotAngle旋转角度，rotAxis旋转角度
    QMatrix4x4 getModelMatrix();

    void draw(GLboolean isOpenLighting = GL_FALSE);
    void processKeyBoard(Car_Movement direction, GLfloat deltaTime);

private:
    LoadMapData *m_map;

    /*********** 车辆参数 *************/
    Model *model;              //车辆模型
    QVector3D position;        //车辆位置
    QVector3D front;           //车辆前进方向
    GLfloat movementSpeed;     //车辆前进速度
    GLfloat turnSpeed;         //车辆转弯速度
    GLfloat yaw;               //车辆偏转角
    QMatrix4x4 originModel;    //车辆模型的初始参数控制，矫正初始模型的大小，平移与旋转
};


class Model
{
public:
    Model();
    bool init(const QString& path);
    void draw(GLboolean isOpenLighting = GL_FALSE);

private:
    bool loadOBJ(const QString& path);
    void bindBufferData();
    QOpenGLFunctions_3_3_Core *core;
    QVector<Object> objects;
    QMap<QString, Material> map_materials;

};

class Object
{
public:
    GLuint positionVBO;
    GLuint uvVBO;
    GLuint normalVBO;

    QVector<QVector3D> positions;
    QVector<QVector2D> uvs;
    QVector<QVector3D> normals;

    QString matName;//材质名称
};

//材质类
class Material
{
public:
    QVector3D Ka;//ambient反射系数
    QVector3D Kd;//diffuse反射系数
    QVector3D Ks;//specular反射系数
    //  QOpenGLTexture* map_Ka;//环境反射贴图
    //  QOpenGLTexture* map_Kd;//漫反射贴图
    double shininess;
    QString name_map_Ka;
    QString name_map_Kd;
};

#endif // CARCONTROL_H
