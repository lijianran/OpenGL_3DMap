#include "openglwindow.h"
#include "loadmapdata.h"
#include "carcontrol.h"

const QVector3D CAMERA_POSITION(2.0f, 2.0f, -20.0f);  //相机初始化位置
const QVector3D LIGHT_POSITION(0.0f, 0.0f, 0.0f);  //灯光位置
const QVector3D LIGHT_DIRECTION(0.0f, -0.5f, 1.0f);  //灯光朝向


static LoadMapData *map;  //数据
static Light *light;  //灯光
static Coordinate *coordinate;  //坐标
static Image *image;  //纹理
static CarControl control;  //车辆

//QVector<WayDatas> tempways;


OpenGLWindow::OpenGLWindow(QWidget *parent, QString filename) : QOpenGLWidget(parent)
{
    //开启抗锯齿
    m_filename = filename;
    QSurfaceFormat newGLFormat = this->format();
    newGLFormat.setSamples(4);
    this->setFormat(newGLFormat);
}

OpenGLWindow::~OpenGLWindow()
{
    if(camera)
        delete camera;
}

void OpenGLWindow::KeyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    if(key <= 1024)
        this->keys[key] = GL_TRUE;
}

void OpenGLWindow::KeyReleaseEvent(QKeyEvent *event)
{
    int key = event->key();
    if(key <= 1024)
        this->keys[key] = GL_FALSE;
}

void OpenGLWindow::OpenMapFile(QString path)
{
    map->init(path);
    map->initGL();
    this->updateGL();
}

void OpenGLWindow::Recovery()
{
    camera->recoverCamera();
}

void OpenGLWindow::initializeGL()
{
    /***********OpenGL***********/
    core = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
    core->glEnable(GL_DEPTH_TEST);
    core->glEnable(GL_MULTISAMPLE);

    /************背景颜色***********/
    core->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    core->glClearColor(0.7f, 0.7f, 0.7f, 1.0f);

    /***********初始化模型细节参数*************/
    isOpenLighting = GL_TRUE;
    isLineMode = GL_FALSE;

    /***********键鼠响应及时间帧数操作*************/
    for(int i = 0; i != 1024; i++)
        keys[i] = GL_FALSE;

    deltaTime = 0.0f;
    lastFrame = 0.0f;

    isFirstMouse = GL_TRUE;
    isLeftMousePress = GL_FALSE;
    lastX = GLint(width() / 2.0f);
    lastY = GLint(height() / 2.0f);

    time.start();

    /************摄像机***********/
    camera = new Camera(CAMERA_POSITION);

    /************2D纹理***********/
    image = new Image();
    image->init();

    /***********灯*************/
    light = new Light();
    light->init();

    /***********坐标系*************/
    coordinate = new Coordinate();
    coordinate->init();

    /************地图数据***********/
    map = new LoadMapData();
    map->init(m_filename);
    map->initGL();

    /************ CarControl汽车控制 ***********/
    control.setMap(map);
    control.setCarModel(":/res/models/jeep/jeep.obj");
    control.setCarOriginMatrix(QVector3D(170.0f, 35.0f, 0.0f), QVector3D(0.001f, 0.001f, 0.001f),-90.0f);

    //载入
    {
        ResourceManager::loadShader("light", ":/res/shaders/light.vert", ":/res/shaders/light.frag");
        ResourceManager::loadShader("coordinate", ":/res/shaders/coordinate.vert", ":/res/shaders/coordinate.frag");
        ResourceManager::loadShader("osm_highway", ":/res/shaders/osm_highway.vert", ":/res/shaders/osm_highway.frag");
        ResourceManager::loadShader("normal_test", ":/res/shaders/normal.vert", ":/res/shaders/normal.frag", ":/res/shaders/normal.geom");
        ResourceManager::loadShader("osm_building", ":/res/shaders/osm_building.vert", ":/res/shaders/osm_building.frag");
        ResourceManager::loadShader("image2D", ":/res/shaders/Image2D.vert", ":/res/shaders/Image2D.frag");
        ResourceManager::loadShader("osm_amenity", ":/res/shaders/osm_amenity.vert", ":/res/shaders/osm_amenity.frag");
        ResourceManager::loadShader("osm_leisure", ":/res/shaders/osm_leisure.vert", ":/res/shaders/osm_leisure.frag");
        ResourceManager::loadShader("osm_area", ":/res/shaders/osm_area.vert", ":/res/shaders/osm_area.frag");
        ResourceManager::loadShader("osm_natural", ":/res/shaders/osm_natural.vert", ":/res/shaders/osm_natural.frag");
        ResourceManager::loadShader("osm_water", ":/res/shaders/osm_water.vert", ":/res/shaders/osm_water.frag");
        ResourceManager::loadShader("osm_landuse", ":/res/shaders/osm_landuse.vert", ":/res/shaders/osm_landuse.frag");
        ResourceManager::loadShader("model", ":/res/shaders/model.vert", ":/res/shaders/model.frag");

        ResourceManager::loadTexture("osm_highway", ":/res/textures/highway.png");
        ResourceManager::loadTexture("osm_building", ":/res/textures/red.png");
        ResourceManager::loadTexture("osm_background", ":/res/textures/background.png");
        ResourceManager::loadTexture("osm_amenity", ":/res/textures/amenity.png");
        ResourceManager::loadTexture("osm_leisure", ":/res/textures/leisure.png");
        ResourceManager::loadTexture("osm_area", ":/res/textures/area.png");
        ResourceManager::loadTexture("osm_natural", ":/res/textures/natural.png");
        ResourceManager::loadTexture("osm_water", ":/res/textures/water.png");
        ResourceManager::loadTexture("osm_landuse", ":/res/textures/landuse.png");

    }

    //设置
    {
        ResourceManager::getShader("model").use().setInteger("material.ambientMap", 0);
        ResourceManager::getShader("model").use().setInteger("material.diffuseMap", 1);
        ResourceManager::getShader("model").use().setVector3f("light.direction", LIGHT_DIRECTION);

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
    }

    {
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
        qDebug() << "maxPoint: x:" << map->maxPoint.x << " y:" << map->maxPoint.y
                 << " minPoint: x:" << map->minPoint.x << " y:" << map->minPoint.y;
        model.scale(100.0f, 0.005f, 100.0f);
        model.translate(-map->minPoint.x, 0, map->minPoint.y);
        ResourceManager::getShader("osm_highway").use().setMatrix4f("model", model);
        ResourceManager::getShader("normal_test").use().setMatrix4f("model", model);

        model.setToIdentity();
        model.scale(100.0f, 0.12f, 100.0f);
        model.translate(-map->minPoint.x, 0, map->minPoint.y);
        ResourceManager::getShader("osm_building").use().setMatrix4f("model", model);

        model.setToIdentity();
        model.scale(100.0f, 0.18f, 100.0f);
        model.translate(-map->minPoint.x, 0, map->minPoint.y);
        ResourceManager::getShader("osm_amenity").use().setMatrix4f("model", model);

        model.setToIdentity();
        model.scale(100.0f, 0.003f, 100.0f);
        model.translate(-map->minPoint.x, 0, map->minPoint.y);
        ResourceManager::getShader("osm_leisure").use().setMatrix4f("model", model);

        model.setToIdentity();
        model.scale(100.0f, 0.001f, 100.0f);
        model.translate(-map->minPoint.x, 0, map->minPoint.y);
        ResourceManager::getShader("osm_area").use().setMatrix4f("model", model);

        model.setToIdentity();
        model.scale(100.0f, 0.003f, 100.0f);
        model.translate(-map->minPoint.x, 0, map->minPoint.y);
        ResourceManager::getShader("osm_water").use().setMatrix4f("model", model);

        model.setToIdentity();
        model.scale(100.0f, 0.003f, 100.0f);
        model.translate(-map->minPoint.x, 0, map->minPoint.y);
        ResourceManager::getShader("osm_natural").use().setMatrix4f("model", model);

        model.setToIdentity();
        model.scale(100.0f, 0.003f, 100.0f);
        model.translate(-map->minPoint.x, 0, map->minPoint.y);
        ResourceManager::getShader("osm_landuse").use().setMatrix4f("model", model);

        model.setToIdentity();
        model.translate(0.0f, -0.05f, 0.0f);
        model.scale(30.0f);
        ResourceManager::getShader("image2D").use().setMatrix4f("model", model);
    }

    /************  小车参数 略复杂 待删除！！！  ******************/
    //  QVector3D trans;
    //  QMap<QString, OpenSMWay>::iterator iter = map->map_Ways.begin();
    //  while(iter != map->map_Ways.end()){
    //    if(iter.value().kvPairs.contains("highway")){
    //      tempways.push_back(iter.value());
    //      QPointD temp = map->map_Nodes[iter.value().nodesID[0]];
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

}

void OpenGLWindow::resizeGL(int w, int h)
{
    core->glViewport(0, 0, w, h);
}

void OpenGLWindow::paintGL()
{
    /*********** 计算两次帧数之间的时间间隔 ***************/
    GLfloat currentFrame = static_cast<GLfloat>(time.elapsed())/100;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    this->processInput(deltaTime);
    this->updateGL();

    /********* 绘制osm 背景 ************/
    core->glActiveTexture(GL_TEXTURE0);
    ResourceManager::getTexture("osm_background").bind();
    ResourceManager::getShader("image2D").use();
    image->draw();

    /********* 绘制坐标系统 ************/
    ResourceManager::getShader("coordinate").use();
    coordinate->draw();

    /********* 绘制车 ************/
    ResourceManager::getShader("model").use();
    control.draw(this->isOpenLighting);

    /********* 绘制 highway ************/
    core->glActiveTexture(GL_TEXTURE0);
    ResourceManager::getTexture("osm_highway").bind();
    core->glActiveTexture(GL_TEXTURE1);
    ResourceManager::getTexture("osm_highway").bind();

    ResourceManager::getShader("osm_highway").use();
    map->drawGL_Highway();

    /********* 绘制 building ************/
    core->glActiveTexture(GL_TEXTURE0);
    ResourceManager::getTexture("osm_building").bind();
    core->glActiveTexture(GL_TEXTURE1);
    ResourceManager::getTexture("osm_building").bind();

    ResourceManager::getShader("osm_building").use();
    map->drawGL_Building();

    /********* 绘制 amentiy ************/
    core->glActiveTexture(GL_TEXTURE0);
    ResourceManager::getTexture("osm_amenity").bind();
    core->glActiveTexture(GL_TEXTURE1);
    ResourceManager::getTexture("osm_amenity").bind();

    ResourceManager::getShader("osm_amenity").use();
    map->drawGL_Amenity();

    /********* 绘制 leisure ************/
    core->glActiveTexture(GL_TEXTURE0);
    ResourceManager::getTexture("osm_leisure").bind();
    core->glActiveTexture(GL_TEXTURE1);
    ResourceManager::getTexture("osm_leisure").bind();

    ResourceManager::getShader("osm_leisure").use();
    map->drawGL_Leisure();

    /********* 绘制 area ************/
    core->glActiveTexture(GL_TEXTURE0);
    ResourceManager::getTexture("osm_area").bind();
    core->glActiveTexture(GL_TEXTURE1);
    ResourceManager::getTexture("osm_area").bind();

    ResourceManager::getShader("osm_area").use();
    map->drawGL_Area();

    /********* 绘制 osm water ************/
    core->glActiveTexture(GL_TEXTURE0);
    ResourceManager::getTexture("osm_water").bind();
    core->glActiveTexture(GL_TEXTURE1);
    ResourceManager::getTexture("osm_water").bind();

    ResourceManager::getShader("osm_water").use();
    map->drawGL_Water();

    /********* 绘制 natural ************/
    core->glActiveTexture(GL_TEXTURE0);
    ResourceManager::getTexture("osm_natural").bind();
    core->glActiveTexture(GL_TEXTURE1);
    ResourceManager::getTexture("osm_natural").bind();

    ResourceManager::getShader("osm_natural").use();
    map->drawGL_Natural();

    /********* 绘制 landuse ************/
    core->glActiveTexture(GL_TEXTURE0);
    ResourceManager::getTexture("osm_landuse").bind();
    core->glActiveTexture(GL_TEXTURE1);
    ResourceManager::getTexture("osm_landuse").bind();

    ResourceManager::getShader("osm_landuse").use();
    map->drawGL_Landuse();

    /********* 恢复重新绘图 ************/
    map->drawGL_Recover();
}

void OpenGLWindow::updateGL()
{
    if(this->isLineMode)
        core->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        core->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    QMatrix4x4 projection;
    projection.perspective(camera->zoom, static_cast<GLfloat>(width())/static_cast<GLfloat>(height()), 0.01f, 200.f);

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



    /************* 小车临时变量 待删除  *************/
    ResourceManager::getShader("model").use().setMatrix4f("model", control.getModelMatrix());

    //  if(temp_k < tempways.size()){
    //  QPointD temp_cur = map->map_Nodes[tempways[temp_k].nodesID[temp_j]];
    //  QPointD temp_next = map->map_Nodes[tempways[temp_k].nodesID[temp_j+1]];
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

void OpenGLWindow::processInput(GLfloat dt)
{
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
    if (keys[Qt::Key_I])
        control.processKeyboard(CAR_STRAIGHT, dt);
    if (keys[Qt::Key_K])
        control.processKeyboard(CAR_BACKWARD, dt);
    if (keys[Qt::Key_J])
        control.processKeyboard(CAR_LEFT, dt);
    if (keys[Qt::Key_L])
        control.processKeyboard(CAR_RIGHT, dt);
}

void OpenGLWindow::mouseMoveEvent(QMouseEvent *event)
{
    GLint xpos = event->pos().x();
    GLint ypos = event->pos().y();
    if(isLeftMousePress)
    {
        if (isFirstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            isFirstMouse = GL_FALSE;
        }
        GLint xoffset = xpos - lastX;
        GLint yoffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;
        camera->processMouseMovement(xoffset, yoffset);
    }
}

void OpenGLWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        isLeftMousePress = GL_TRUE;
}

void OpenGLWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        isLeftMousePress = GL_FALSE;
        isFirstMouse = GL_TRUE;
    }
}

void OpenGLWindow::wheelEvent(QWheelEvent *event)
{
    QPoint offset = event->angleDelta();
    camera->processMouseScroll(offset.y()/20.0f);
}

/************** 相机 ************************/

QMatrix4x4 Camera::getViewMatrix()
{
    QMatrix4x4 view;
    view.lookAt(this->position, this->position + this->front, this->up);
    return view;
}

//键盘
void Camera::processKeyboard(Camera_Movement direction, GLfloat deltaTime)
{
    GLfloat velocity = this->movementSpeed * deltaTime;
    if (direction == CAMERA_FORWARD)
        this->position += this->front * velocity;
    if (direction == CAMERA_BACKWARD)
        this->position -= this->front * velocity;
    if (direction == CAMERA_LEFT)
        this->position -= this->right * velocity;
    if (direction == CAMERA_RIGHT)
        this->position += this->right * velocity;
    if (direction == CAMERA_UP)
        this->position += this->worldUp * velocity;
    if (direction == CAMERA_DOWN)
        this->position -= this->worldUp * velocity;
}

//恢复
void Camera::recoverCamera()
{
    this->position = QVector3D(0.0f, 0.0f, 0.0f);
    this->worldUp = QVector3D(0.0f, 1.0f, 0.0f);
    this->yaw = 0.0f;
    this->picth = 0.0f;
    this->updateCameraVectors();
}

//鼠标
void Camera::processMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constraintPitch)
{
    xoffset *= this->mouseSensitivity;
    yoffset *= this->mouseSensitivity;

    this->yaw += xoffset;
    this->picth += yoffset;

    if (constraintPitch)
    {
        if (this->picth > 89.0f)
            this->picth = 89.0f;
        if (this->picth < -89.0f)
            this->picth = -89.0f;
    }

    this->updateCameraVectors();
}

//滑轮
void Camera::processMouseScroll(GLfloat yoffset)
{
    if (this->zoom >= 1.0f && this->zoom <= 45.0f)
        this->zoom -= yoffset;
    if (this->zoom > 45.0f)
        this->zoom = 45.0f;
    if (this->zoom < 1.0f)
        this->zoom = 1.0f;
}

void Camera::updateCameraVectors()
{
    float yawR = qDegreesToRadians(this->yaw);
    float picthR = qDegreesToRadians(this->picth);

    QVector3D front3(float((cos(double(yawR))) * cos(double(picthR))),
                     float((sin(double(picthR)))),
                     float((sin(double(yawR)) * cos(double(picthR)))));
    this->front = front3.normalized();
    this->right = QVector3D::crossProduct(this->front, this->worldUp).normalized();
    this->up = QVector3D::crossProduct(this->right, this->front).normalized();
}

/**************灯光************************/

Light::Light(): VBO(0)
{
    core = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
}

Light::~Light()
{
    if(VBO != 0)
        core->glDeleteBuffers(1, &VBO);
}

void Light::init()
{
    float lightVertices[] = {
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f
    };

    core->glGenBuffers(1, &VBO);

    core->glBindBuffer(GL_ARRAY_BUFFER, VBO);
    core->glBufferData(GL_ARRAY_BUFFER, sizeof(lightVertices), lightVertices, GL_STATIC_DRAW);
}

void Light::draw()
{
    core->glEnableVertexAttribArray(0);
    core->glBindBuffer(GL_ARRAY_BUFFER, VBO);
    core->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    core->glDrawArrays(GL_TRIANGLES, 0, 36);
}

/**************坐标系************************/

Coordinate::Coordinate(): VBO(0)
{
    core = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
}

Coordinate::~Coordinate()
{
    if(VBO != 0)
        core->glDeleteBuffers(1, &VBO);
}

void Coordinate::init()
{
    float vertices[] = {
        // positions          // normals           // texture coords
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,

        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    core->glGenBuffers(1, &VBO);

    core->glBindBuffer(GL_ARRAY_BUFFER, VBO);
    core->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}

void Coordinate::draw()
{
    core->glEnableVertexAttribArray(0);
    core->glBindBuffer(GL_ARRAY_BUFFER, VBO);
    core->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    core->glDrawArrays(GL_LINES, 0, 6);
}

/**************纹理************************/

Image::Image(): positionVBO(0)
{
    core = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
}

Image::~Image()
{
    if(positionVBO != 0)
        core->glDeleteBuffers(1, &positionVBO);
}

void Image::init()
{
    QVector<QVector3D> positions;
    positions.push_back(QVector3D(-0.5f, 0.0f, -0.5f));
    positions.push_back(QVector3D(0.5f, 0.0f, -0.5f));
    positions.push_back(QVector3D(0.5f, 0.0f, 0.5f));
    positions.push_back(QVector3D(-0.5f, 0.0f, 0.5f));

    core->glGenBuffers(1, &positionVBO);
    core->glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    core->glBufferData(GL_ARRAY_BUFFER, positions.size()* int(sizeof(QVector3D)), &positions[0], GL_STATIC_DRAW);

    QVector<QVector2D> uvs;
    uvs.push_back(QVector2D(0, 0));
    uvs.push_back(QVector2D(1, 0));
    uvs.push_back(QVector2D(1, 1));
    uvs.push_back(QVector2D(0, 1));

    core->glGenBuffers(1, &VBO);
    core->glBindBuffer(GL_ARRAY_BUFFER, VBO);
    core->glBufferData(GL_ARRAY_BUFFER, uvs.size() * int(sizeof(QVector2D)), &uvs[0], GL_STATIC_DRAW);
}

void Image::draw()
{
    core->glEnableVertexAttribArray(0);
    core->glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    core->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    core->glEnableVertexAttribArray(1);
    core->glBindBuffer(GL_ARRAY_BUFFER, VBO);
    core->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    core->glDrawArrays(GL_QUADS, 0, 4);
}
