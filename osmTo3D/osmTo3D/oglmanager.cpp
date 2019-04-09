#include "oglmanager.h"

#include <QDebug>
#include <QPalette>

#include "model.h"
#include "light.h"
#include "resourcemanager.h"
#include "opensmloading.h"
#include "carcontrol.h"

const QVector3D CAMERA_POSITION(0.0f, 0.0f, 3.0f);
const QVector3D LIGHT_POSITION(0.0f, 0.0f, 0.0f);
//const QVector3D LIGHT_DIRECTION(0.0f, -0.5f, -1.0f);
const QVector3D LIGHT_DIRECTION(0.0f, -0.5f, 1.0f);

const int OGLMANAGER_WIDTH = 600;
const int OGLMANAGER_HEIGHT = 600;

OpenSMLoading *osm_GL;//数据
Light *light;//灯光
Coordinate *coordinate;//坐标
Image *image;//纹理
CarControl control;//车辆

//QVector<OpenSMWay> tempways;

OGLManager::OGLManager(QWidget *parent) : QOpenGLWidget(parent){
    this->setGeometry(10, 20, OGLMANAGER_WIDTH, OGLMANAGER_HEIGHT);

    QSurfaceFormat newGLFormat = this->format();  //开启抗锯齿
    newGLFormat.setSamples(4);
    this->setFormat(newGLFormat);

}

OGLManager::~OGLManager(){
    if(camera)
        delete camera;

}

void OGLManager::handleKeyPressEvent(QKeyEvent *event){
    GLuint key = event->key();
    if(key <= 1024)
        this->keys[key] = GL_TRUE;

}

void OGLManager::handleKeyReleaseEvent(QKeyEvent *event){
    GLuint key = event->key();
    if(key <= 1024)
        this->keys[key] = GL_FALSE;
}

//初始化
void OGLManager::initializeGL(){
    /*********** OGL核心 ***********/
    core = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
    core->glEnable(GL_DEPTH_TEST);
    core->glEnable(GL_MULTISAMPLE);

    /*********** 初始化模型细节参数  *************/
    isOpenLighting = GL_TRUE;
    isLineMode = GL_FALSE;

    /*********** 键鼠响应及时间帧数操作  *************/
    for(GLuint i = 0; i != 1024; ++i) //初始化键盘按键
        keys[i] = GL_FALSE;

    deltaTime = 0.0f;
    lastFrame = 0.0f;

    isFirstMouse = GL_TRUE;
    isLeftMousePress = GL_FALSE;
    lastX = width() / 2.0f;
    lastY = height() / 2.0f;

    time.start();

    /************ 摄像机 ***********/
    camera = new Camera(CAMERA_POSITION);

    /************ image2D 纹理 ***********/
    image = new Image();
    image->init();

    /*********** 灯  *************/
    //  light = new Light();
    //  light->init();

    coordinate = new Coordinate();
    coordinate->init();

    /************ openstreetmap ***********/
    osm_GL = new OpenSMLoading();
    osm_GL->init("C:/Users/lijianran/Desktop/2.osm");
    osm_GL->initGL();

    /************ CarControl汽车控制 ***********/
    control.setOSM(osm_GL);
    control.setCarModel("C:/Users/lijianran/Desktop/osmTo3D/osmTo3D/res/models/jeep/jeep.obj");
    control.setCarOriginMatrix(QVector3D(170.0f, 35.0f, 0.0f), QVector3D(0.001f, 0.001f, 0.001f),
                               -90.0f);

    /************ 载入shader ***********/
    ResourceManager::loadShader("light", ":/shaders/res/shaders/light.vert", ":/shaders/res/shaders/light.frag");
    ResourceManager::loadShader("coordinate", ":/shaders/res/shaders/coordinate.vert", ":/shaders/res/shaders/coordinate.frag");
    ResourceManager::loadShader("osm_highway", ":/shaders/res/shaders/osm_highway.vert", ":/shaders/res/shaders/osm_highway.frag");
    ResourceManager::loadShader("normal_test", ":/shaders/res/shaders/normal.vert", ":/shaders/res/shaders/normal.frag", ":/shaders/res/shaders/normal.geom");
    ResourceManager::loadShader("osm_building", ":/shaders/res/shaders/osm_building.vert", ":/shaders/res/shaders/osm_building.frag");
    ResourceManager::loadShader("image2D", ":/shaders/res/shaders/Image2D.vert", ":/shaders/res/shaders/Image2D.frag");
    ResourceManager::loadShader("osm_amenity", ":/shaders/res/shaders/osm_amenity.vert", ":/shaders/res/shaders/osm_amenity.frag");
    ResourceManager::loadShader("osm_leisure", ":/shaders/res/shaders/osm_leisure.vert", ":/shaders/res/shaders/osm_leisure.frag");
    ResourceManager::loadShader("osm_area", ":/shaders/res/shaders/osm_area.vert", ":/shaders/res/shaders/osm_area.frag");
    ResourceManager::loadShader("osm_natural", ":/shaders/res/shaders/osm_natural.vert", ":/shaders/res/shaders/osm_natural.frag");
    ResourceManager::loadShader("osm_water", ":/shaders/res/shaders/osm_water.vert", ":/shaders/res/shaders/osm_water.frag");
    ResourceManager::loadShader("osm_landuse", ":/shaders/res/shaders/osm_landuse.vert", ":/shaders/res/shaders/osm_landuse.frag");
    ResourceManager::loadShader("model", ":/shaders/res/shaders/model.vert", ":/shaders/res/shaders/model.frag");

    ResourceManager::getShader("model").use().setInteger("material.ambientMap", 0);
    ResourceManager::getShader("model").use().setInteger("material.diffuseMap", 1);
    ResourceManager::getShader("model").use().setVector3f("light.direction", LIGHT_DIRECTION);

    //ResourceManager::loadTexture("osm_highway", ":/textures/res/textures/black.png");
    ResourceManager::loadTexture("osm_highway", ":/textures/res/textures/highway.png");
    ResourceManager::loadTexture("osm_building", ":/textures/res/textures/red.png");
    ResourceManager::loadTexture("osm_background", ":/textures/res/textures/background.png");
    //ResourceManager::loadTexture("osm_background", ":/textures/res/textures/black.png");

    ResourceManager::loadTexture("osm_amenity", ":/textures/res/textures/amenity.png");
    ResourceManager::loadTexture("osm_leisure", ":/textures/res/textures/leisure.png");
    ResourceManager::loadTexture("osm_area", ":/textures/res/textures/area.png");
    ResourceManager::loadTexture("osm_natural", ":/textures/res/textures/natural.png");
    ResourceManager::loadTexture("osm_water", ":/textures/res/textures/water.png");
    ResourceManager::loadTexture("osm_landuse", ":/textures/res/textures/landuse.png");


    ResourceManager::getShader("osm_highway").use().setInteger("material.ambientMap", 0);
    ResourceManager::getShader("osm_highway").use().setInteger("material.specularMap", 1);
    ResourceManager::getShader("osm_highway").use().setFloat("material.shininess", 64.0f);
    ResourceManager::getShader("osm_highway").use().setVector3f("material.Ka", QVector3D(0.3f, 0.3f, 0.3f));
    ResourceManager::getShader("osm_highway").use().setVector3f("material.Kd", QVector3D(0.6f, 0.6f, 0.6f));
    ResourceManager::getShader("osm_highway").use().setVector3f("material.Ks", QVector3D(0.2f, 0.2f, 0.2f));
    ResourceManager::getShader("osm_highway").use().setVector3f("light.direction", LIGHT_DIRECTION);

    ResourceManager::getShader("osm_building").use().setInteger("material.ambientMap", 0);
    ResourceManager::getShader("osm_building").use().setInteger("material.specularMap", 1);
    ResourceManager::getShader("osm_building").use().setFloat("material.shininess", 64.0f);
    ResourceManager::getShader("osm_building").use().setVector3f("material.Ka", QVector3D(0.2f, 0.2f, 0.2f));
    ResourceManager::getShader("osm_building").use().setVector3f("material.Kd", QVector3D(0.5f, 0.5f, 0.5f));
    ResourceManager::getShader("osm_building").use().setVector3f("material.Ks", QVector3D(1.0f, 1.0f, 1.0f));
    ResourceManager::getShader("osm_building").use().setVector3f("light.direction", LIGHT_DIRECTION);

    ResourceManager::getShader("osm_amenity").use().setInteger("material.ambientMap", 0);
    ResourceManager::getShader("osm_amenity").use().setInteger("material.specularMap", 1);
    ResourceManager::getShader("osm_amenity").use().setFloat("material.shininess", 64.0f);
    ResourceManager::getShader("osm_amenity").use().setVector3f("material.Ka", QVector3D(0.2f, 0.2f, 0.2f));
    ResourceManager::getShader("osm_amenity").use().setVector3f("material.Kd", QVector3D(0.5f, 0.5f, 0.5f));
    ResourceManager::getShader("osm_amenity").use().setVector3f("material.Ks", QVector3D(1.0f, 1.0f, 1.0f));
    ResourceManager::getShader("osm_amenity").use().setVector3f("light.direction", LIGHT_DIRECTION);

    ResourceManager::getShader("osm_leisure").use().setInteger("material.ambientMap", 0);
    ResourceManager::getShader("osm_leisure").use().setInteger("material.specularMap", 1);
    ResourceManager::getShader("osm_leisure").use().setFloat("material.shininess", 64.0f);
    ResourceManager::getShader("osm_leisure").use().setVector3f("material.Ka", QVector3D(0.2f, 0.2f, 0.2f));
    ResourceManager::getShader("osm_leisure").use().setVector3f("material.Kd", QVector3D(0.5f, 0.5f, 0.5f));
    ResourceManager::getShader("osm_leisure").use().setVector3f("material.Ks", QVector3D(1.0f, 1.0f, 1.0f));
    ResourceManager::getShader("osm_leisure").use().setVector3f("light.direction", LIGHT_DIRECTION);

    ResourceManager::getShader("osm_area").use().setInteger("material.ambientMap", 0);
    ResourceManager::getShader("osm_area").use().setInteger("material.specularMap", 1);
    ResourceManager::getShader("osm_area").use().setFloat("material.shininess", 64.0f);
    ResourceManager::getShader("osm_area").use().setVector3f("material.Ka", QVector3D(0.7f, 0.7f, 0.7f));
    ResourceManager::getShader("osm_area").use().setVector3f("material.Kd", QVector3D(0.2f, 0.2f, 0.2f));
    ResourceManager::getShader("osm_area").use().setVector3f("material.Ks", QVector3D(0.1f, 0.1f, 0.1f));
    ResourceManager::getShader("osm_area").use().setVector3f("light.direction", LIGHT_DIRECTION);

    ResourceManager::getShader("osm_natural").use().setInteger("material.ambientMap", 0);
    ResourceManager::getShader("osm_natural").use().setInteger("material.specularMap", 1);
    ResourceManager::getShader("osm_natural").use().setFloat("material.shininess", 64.0f);
    ResourceManager::getShader("osm_natural").use().setVector3f("material.Ka", QVector3D(0.7f, 0.7f, 0.7f));
    ResourceManager::getShader("osm_natural").use().setVector3f("material.Kd", QVector3D(0.2f, 0.2f, 0.2f));
    ResourceManager::getShader("osm_natural").use().setVector3f("material.Ks", QVector3D(0.1f, 0.1f, 0.1f));
    ResourceManager::getShader("osm_natural").use().setVector3f("light.direction", LIGHT_DIRECTION);

    ResourceManager::getShader("osm_water").use().setInteger("material.ambientMap", 0);
    ResourceManager::getShader("osm_water").use().setInteger("material.specularMap", 1);
    ResourceManager::getShader("osm_water").use().setFloat("material.shininess", 64.0f);
    ResourceManager::getShader("osm_water").use().setVector3f("material.Ka", QVector3D(0.7f, 0.7f, 0.7f));
    ResourceManager::getShader("osm_water").use().setVector3f("material.Kd", QVector3D(0.2f, 0.2f, 0.2f));
    ResourceManager::getShader("osm_water").use().setVector3f("material.Ks", QVector3D(0.1f, 0.1f, 0.1f));
    ResourceManager::getShader("osm_water").use().setVector3f("light.direction", LIGHT_DIRECTION);

    ResourceManager::getShader("osm_landuse").use().setInteger("material.ambientMap", 0);
    ResourceManager::getShader("osm_landuse").use().setInteger("material.specularMap", 1);
    ResourceManager::getShader("osm_landuse").use().setFloat("material.shininess", 64.0f);
    ResourceManager::getShader("osm_landuse").use().setVector3f("material.Ka", QVector3D(0.7f, 0.7f, 0.7f));
    ResourceManager::getShader("osm_landuse").use().setVector3f("material.Kd", QVector3D(0.2f, 0.2f, 0.2f));
    ResourceManager::getShader("osm_landuse").use().setVector3f("material.Ks", QVector3D(0.1f, 0.1f, 0.1f));
    ResourceManager::getShader("osm_landuse").use().setVector3f("light.direction", LIGHT_DIRECTION);


    ResourceManager::getShader("image2D").use().setInteger("ambientMap", 0);

    QMatrix4x4 model;
    model.translate(LIGHT_POSITION);
    model.scale(0.1f);
    ResourceManager::getShader("light").use().setMatrix4f("model", model);

    //  model.setToIdentity();
    //  model.scale(0.0003f);
    //  model.translate(170.0f, 30.0f, 0);
    //  ResourceManager::getShader("model").use().setMatrix4f("model", model);

    model.setToIdentity();
    model.scale(20.0f);
    ResourceManager::getShader("coordinate").use().setMatrix4f("model", model);

    model.setToIdentity();
    qDebug() << "maxPoint: x:" << osm_GL->maxPoint.x << " y:" << osm_GL->maxPoint.y
             << " minPoint: x:" << osm_GL->minPoint.x << " y:" << osm_GL->minPoint.y;
    model.scale(100.0f, 0.005f, 100.0f);
    model.translate(-osm_GL->minPoint.x, 0, osm_GL->minPoint.y);
    ResourceManager::getShader("osm_highway").use().setMatrix4f("model", model);
    ResourceManager::getShader("normal_test").use().setMatrix4f("model", model);


    model.setToIdentity();
    model.scale(100.0f, 0.12f, 100.0f);
    model.translate(-osm_GL->minPoint.x, 0, osm_GL->minPoint.y);
    ResourceManager::getShader("osm_building").use().setMatrix4f("model", model);

    model.setToIdentity();
    model.scale(100.0f, 0.18f, 100.0f);
    model.translate(-osm_GL->minPoint.x, 0, osm_GL->minPoint.y);
    ResourceManager::getShader("osm_amenity").use().setMatrix4f("model", model);

    model.setToIdentity();
    model.scale(100.0f, 0.003f, 100.0f);
    model.translate(-osm_GL->minPoint.x, 0, osm_GL->minPoint.y);
    ResourceManager::getShader("osm_leisure").use().setMatrix4f("model", model);

    model.setToIdentity();
    model.scale(100.0f, 0.001f, 100.0f);
    model.translate(-osm_GL->minPoint.x, 0, osm_GL->minPoint.y);
    ResourceManager::getShader("osm_area").use().setMatrix4f("model", model);

    model.setToIdentity();
    model.scale(100.0f, 0.003f, 100.0f);
    model.translate(-osm_GL->minPoint.x, 0, osm_GL->minPoint.y);
    ResourceManager::getShader("osm_water").use().setMatrix4f("model", model);

    model.setToIdentity();
    model.scale(100.0f, 0.003f, 100.0f);
    model.translate(-osm_GL->minPoint.x, 0, osm_GL->minPoint.y);
    ResourceManager::getShader("osm_natural").use().setMatrix4f("model", model);

    model.setToIdentity();
    model.scale(100.0f, 0.003f, 100.0f);
    model.translate(-osm_GL->minPoint.x, 0, osm_GL->minPoint.y);
    ResourceManager::getShader("osm_landuse").use().setMatrix4f("model", model);



    model.setToIdentity();
    model.translate(0.0f, -0.05f, 0.0f);
    model.scale(30.0f);
    ResourceManager::getShader("image2D").use().setMatrix4f("model", model);

    /************  小车参数 略复杂 待删除！！！  ******************/
    //  QVector3D trans;
    //  QMap<QString, OpenSMWay>::iterator iter = osm_GL->map_Ways.begin();
    //  while(iter != osm_GL->map_Ways.end()){
    //    if(iter.value().kvPairs.contains("highway")){
    //      tempways.push_back(iter.value());
    //      QPointD temp = osm_GL->map_Nodes[iter.value().nodesID[0]];
    //      trans = QVector3D(temp.x, 0, -temp.y);
    //    }
    //    ++iter;
    //  }

    //  model.setToIdentity();
    //  model.scale(100.0f, 1.0f, 100.0f);
    ////  model.translate(-108.75f, 0.01f, 34.03f);
    ////  model.translate(trans);
    //  model.scale(0.000002f, 0.0002f, 0.000002f);
    //  model.translate(170.0f, 35.0f, 0);
    ResourceManager::getShader("model").use().setMatrix4f("model", control.getModelMatrix());


    /************ 背景颜色参数调控 ***********/
    core->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    core->glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    //core->glClearColor(1.7f, 1.7f, 1.7f, 1.0f);
}


void OGLManager::resizeGL(int w, int h){
    core->glViewport(0, 0, w, h);
}

//绘图
void OGLManager::paintGL(){
    /*********** 计算两次帧数之间的时间间隔  ***************/
    GLfloat currentFrame = (GLfloat)time.elapsed()/100;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    this->processInput(deltaTime);
    this->updateGL();

    /*********  绘制osm 背景 ************/
    core->glActiveTexture(GL_TEXTURE0);
    ResourceManager::getTexture("osm_background").bind();
    ResourceManager::getShader("image2D").use();
    image->draw();

    /*********  绘制车 ************/
    ResourceManager::getShader("model").use();
    control.draw(this->isOpenLighting);

    /*********  绘制 osm highway ************/
    core->glActiveTexture(GL_TEXTURE0);
    ResourceManager::getTexture("osm_highway").bind();
    core->glActiveTexture(GL_TEXTURE1);
    ResourceManager::getTexture("osm_highway").bind();

    ResourceManager::getShader("osm_highway").use();
    osm_GL->drawGL_Highway();

    //  ResourceManager::getShader("normal_test").use();
    //  osm_GL->drawGL_Highway();

    /*********  绘制 osm building ************/
    core->glActiveTexture(GL_TEXTURE0);
    ResourceManager::getTexture("osm_building").bind();
    core->glActiveTexture(GL_TEXTURE1);
    ResourceManager::getTexture("osm_building").bind();

    ResourceManager::getShader("osm_building").use();
    osm_GL->drawGL_Building();

    /*********  绘制 osm amentiy ************/
    core->glActiveTexture(GL_TEXTURE0);
    ResourceManager::getTexture("osm_amenity").bind();
    core->glActiveTexture(GL_TEXTURE1);
    ResourceManager::getTexture("osm_amenity").bind();

    ResourceManager::getShader("osm_amenity").use();
    osm_GL->drawGL_Amenity();

    /*********  绘制 osm leisure ************/
    core->glActiveTexture(GL_TEXTURE0);
    ResourceManager::getTexture("osm_leisure").bind();
    core->glActiveTexture(GL_TEXTURE1);
    ResourceManager::getTexture("osm_leisure").bind();

    ResourceManager::getShader("osm_leisure").use();
    osm_GL->drawGL_Leisure();

    /*********  绘制 osm area ************/
    core->glActiveTexture(GL_TEXTURE0);
    ResourceManager::getTexture("osm_area").bind();
    core->glActiveTexture(GL_TEXTURE1);
    ResourceManager::getTexture("osm_area").bind();

    ResourceManager::getShader("osm_area").use();
    osm_GL->drawGL_Area();

    /*********  绘制 osm water ************/
    core->glActiveTexture(GL_TEXTURE0);
    ResourceManager::getTexture("osm_water").bind();
    core->glActiveTexture(GL_TEXTURE1);
    ResourceManager::getTexture("osm_water").bind();

    ResourceManager::getShader("osm_water").use();
    osm_GL->drawGL_Water();

    /*********  绘制 osm natural ************/
    core->glActiveTexture(GL_TEXTURE0);
    ResourceManager::getTexture("osm_natural").bind();
    core->glActiveTexture(GL_TEXTURE1);
    ResourceManager::getTexture("osm_natural").bind();

    ResourceManager::getShader("osm_natural").use();
    osm_GL->drawGL_Natural();

    /*********  绘制 osm landuse ************/
    core->glActiveTexture(GL_TEXTURE0);
    ResourceManager::getTexture("osm_landuse").bind();
    core->glActiveTexture(GL_TEXTURE1);
    ResourceManager::getTexture("osm_landuse").bind();

    ResourceManager::getShader("osm_landuse").use();
    osm_GL->drawGL_Landuse();

    /*********  绘制osm 恢复重新绘图 ************/
    osm_GL->drawGL_Recover();


    /*********  绘制坐标系统 ************/
    ResourceManager::getShader("coordinate").use();
    coordinate->draw();
}


void OGLManager::processInput(GLfloat dt){
    //相机控制
    if (keys[Qt::Key_W])
        camera->processKeyboard(CAMERA_FORWARD, dt);
    if (keys[Qt::Key_S])
        camera->processKeyboard(CAMERA_BACKWARD, dt);
    if (keys[Qt::Key_A])
        camera->processKeyboard(CAMERA_LEFT, dt);
    if (keys[Qt::Key_D])
        camera->processKeyboard(CAMERA_RIGHT, dt);
    if (keys[Qt::Key_E])
        camera->processKeyboard(CAMERA_UP, dt);
    if (keys[Qt::Key_Q])
        camera->processKeyboard(CAMERA_DOWN, dt);
    //车辆控制
    if (keys[Qt::Key_I])            //为什么不用Key_Up 因为，他的十六进制为0x01000013，转为十进制为16777235，已经超过1024
        control.processKeyboard(CAR_STRAIGHT, dt);
    if (keys[Qt::Key_K])
        control.processKeyboard(CAR_BACKWARD, dt);
    if (keys[Qt::Key_J])
        control.processKeyboard(CAR_LEFT, dt);
    if (keys[Qt::Key_L])
        control.processKeyboard(CAR_RIGHT, dt);
}

//int temp_i = 0;
//int temp_j = 0;
//int temp_k = 0;

void OGLManager::updateGL(){
    if(this->isLineMode)
        core->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        core->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    QMatrix4x4 projection, view;
    projection.perspective(camera->zoom, (GLfloat)width()/(GLfloat)height(), 0.01f, 200.f);
    view = camera->getViewMatrix();

    ResourceManager::getShader("light").use().setMatrix4f("projection", projection);
    ResourceManager::getShader("light").use().setMatrix4f("view", camera->getViewMatrix());

    ResourceManager::getShader("model").use().setMatrix4f("projection", projection);
    ResourceManager::getShader("model").use().setMatrix4f("view", camera->getViewMatrix());
    ResourceManager::getShader("model").use().setVector3f("viewPos", camera->position);

    ResourceManager::getShader("image2D").use().setMatrix4f("projection", projection);
    ResourceManager::getShader("image2D").use().setMatrix4f("view", camera->getViewMatrix());

    ResourceManager::getShader("osm_highway").use().setMatrix4f("projection", projection);
    ResourceManager::getShader("osm_highway").use().setMatrix4f("view", camera->getViewMatrix());
    ResourceManager::getShader("osm_highway").use().setVector3f("viewPos", camera->position);


    ResourceManager::getShader("coordinate").use().setMatrix4f("projection", projection);
    ResourceManager::getShader("coordinate").use().setMatrix4f("view", camera->getViewMatrix());

    ResourceManager::getShader("osm_building").use().setMatrix4f("projection", projection);
    ResourceManager::getShader("osm_building").use().setMatrix4f("view", camera->getViewMatrix());
    ResourceManager::getShader("osm_building").use().setVector3f("viewPos", camera->position);

    ResourceManager::getShader("osm_amenity").use().setMatrix4f("projection", projection);
    ResourceManager::getShader("osm_amenity").use().setMatrix4f("view", camera->getViewMatrix());
    ResourceManager::getShader("osm_amenity").use().setVector3f("viewPos", camera->position);

    ResourceManager::getShader("osm_leisure").use().setMatrix4f("projection", projection);
    ResourceManager::getShader("osm_leisure").use().setMatrix4f("view", camera->getViewMatrix());
    ResourceManager::getShader("osm_leisure").use().setVector3f("viewPos", camera->position);

    ResourceManager::getShader("osm_area").use().setMatrix4f("projection", projection);
    ResourceManager::getShader("osm_area").use().setMatrix4f("view", camera->getViewMatrix());
    ResourceManager::getShader("osm_area").use().setVector3f("viewPos", camera->position);

    ResourceManager::getShader("osm_natural").use().setMatrix4f("projection", projection);
    ResourceManager::getShader("osm_natural").use().setMatrix4f("view", camera->getViewMatrix());
    ResourceManager::getShader("osm_natural").use().setVector3f("viewPos", camera->position);

    ResourceManager::getShader("osm_water").use().setMatrix4f("projection", projection);
    ResourceManager::getShader("osm_water").use().setMatrix4f("view", camera->getViewMatrix());
    ResourceManager::getShader("osm_water").use().setVector3f("viewPos", camera->position);

    ResourceManager::getShader("osm_landuse").use().setMatrix4f("projection", projection);
    ResourceManager::getShader("osm_landuse").use().setMatrix4f("view", camera->getViewMatrix());
    ResourceManager::getShader("osm_landuse").use().setVector3f("viewPos", camera->position);

    ResourceManager::getShader("normal_test").use().setMatrix4f("projection", projection);
    ResourceManager::getShader("normal_test").use().setMatrix4f("view", camera->getViewMatrix());



    /************* 小车临时变量 ， 待删除  *************/
    ResourceManager::getShader("model").use().setMatrix4f("model", control.getModelMatrix());

    //  if(temp_k < tempways.size()){
    //  QPointD temp_cur = osm_GL->map_Nodes[tempways[temp_k].nodesID[temp_j]];
    //  QPointD temp_next = osm_GL->map_Nodes[tempways[temp_k].nodesID[temp_j+1]];
    //  QPointD temp_min(temp_next.x-temp_cur.x, temp_next.y-temp_cur.y);
    //  temp_min.x /= 100;
    //  temp_min.y /= 100;
    //  QPointD temp(temp_cur.x + temp_min.x*temp_i, temp_cur.y+temp_min.y*temp_i);
    //  temp_i++;
    //  if(temp_i == 100){
    //      temp_j++;
    //      temp_i = 0;
    //  }
    //  if(temp_j == tempways[temp_k].nodesID.size()-1){
    //      temp_k++;
    //      temp_j = 0;
    //  }

    //  QVector3D  trans;
    //  trans = QVector3D(temp.x, 0, -temp.y);
    //  QMatrix4x4 model;

    //  model.setToIdentity();
    //  model.scale(100.0f, 1.0f, 100.0f);
    //  model.translate(-108.75f, 0.01f, 34.03f);
    //  model.translate(trans);
    //  model.scale(0.000001f, 0.0001f, 0.000001f);
    //  model.translate(170.0f, 35.0f, 0);
    //  ResourceManager::getShader("model").use().setMatrix4f("model", model);

    //  }
}

//鼠标
void OGLManager::mouseMoveEvent(QMouseEvent *event){
    GLint xpos = event->pos().x();
    GLint ypos = event->pos().y();
    if(isLeftMousePress){
        if (isFirstMouse){
            lastX = xpos;
            lastY = ypos;
            isFirstMouse = GL_FALSE;
        }

        GLint xoffset = xpos - lastX;
        GLint yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
        lastX = xpos;
        lastY = ypos;
        camera->processMouseMovement(xoffset, yoffset);

    }
}

void OGLManager::mousePressEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton)//注意是button（）不是buttons（）；
        isLeftMousePress = GL_TRUE;
}

void OGLManager::mouseReleaseEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton){ //注意是button（）不是buttons（）；
        isLeftMousePress = GL_FALSE;
        isFirstMouse = GL_TRUE;
    }
}

//滑轮
void OGLManager::wheelEvent(QWheelEvent *event){
    QPoint offset = event->angleDelta();
    camera->processMouseScroll(offset.y()/20.0f);
}
