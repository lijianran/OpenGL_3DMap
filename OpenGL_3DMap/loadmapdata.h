#ifndef LOADMAPDATA_H
#define LOADMAPDATA_H

#include <QOpenGLFunctions_3_3_Core>
#include <QString>
#include <QVector>
#include <QMap>
#include <QDebug>

struct onePoint
{
    onePoint(): x(0), y(0){}
    onePoint(float x, float y)
    {
        this->x = x;
        this->y = y;
    }
    onePoint(const onePoint& other)
    {
        this->x = other.x;
        this->y = other.y;
    }

    float x;
    float y;
};

class WayDatas
{
public:
    WayDatas(): isDraw(false){}
    ~WayDatas(){}

    QVector<QString> nodesID;
    QMap<QString, QString> kvPairs;

    GLuint positionVBO;
    GLuint uvVBO;
    GLuint normalVBO;

    int supFaceNum;
    bool isDraw;
};

class LoadMapData
{
public:
    LoadMapData();
    ~LoadMapData();
    bool init(const QString &path);
    void initGL();
    void drawGL_Highway(); //way 道路
    void drawGL_Building(); //building 建筑
    void drawGL_Amenity(); //amenity 便利设施
    void drawGL_Leisure(); //leisure 娱乐设施
    void drawGL_Area(); //Area 边界线
    void drawGL_Water();//water 水
    void drawGL_Landuse(); //landuse 草或森林
    void drawGL_Natural(); //natural 自然区域

    void drawGL_Recover();
    QMap<QString, onePoint> map_Nodes;
    QMap<QString, WayDatas> map_Ways;
    onePoint maxPoint;  //该变量存储way中highway的x最大值与y最大值
    onePoint minPoint;  //该变量存储way中highway的x最小值与y最小值
private:
    QOpenGLFunctions_3_3_Core *core;
};

#endif // LOADMAPDATA_H
