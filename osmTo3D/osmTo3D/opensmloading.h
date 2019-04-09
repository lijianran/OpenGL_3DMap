#ifndef OPENSMLOADING_H
#define OPENSMLOADING_H

#include <QOpenGLFunctions_3_3_Core>
#include <QString>
#include <QVector>
#include <QMap>
#include <QDebug>

struct QPointD{
    QPointD(): x(0), y(0){}
    QPointD(double x, double y){
        this->x = x;
        this->y = y;
    }
    QPointD(const QPointD& other){
        this->x = other.x;
        this->y = other.y;
    }

    double x;
    double y;
};

class OpenSMWay{
public:
    OpenSMWay(): isDraw(false){}
    ~OpenSMWay(){
    }

    QVector<QString> nodesID;
    QMap<QString, QString> kvPairs;

    GLuint positionVBO;
    GLuint uvVBO;
    GLuint normalVBO;

    int supFaceNum;// supple,增补，如果是建筑或其他多边形，需要增补的 三角剖分化后的面或底面的点的数量
    bool isDraw;
};

class OpenSMLoading{
public:
    OpenSMLoading();
    ~OpenSMLoading();
    bool init(const QString &path);
    void initGL();
    void drawGL_Highway();
    void drawGL_Building();
    void drawGL_Amenity(); //amenity 便利设施
    void drawGL_Leisure(); //leisure 娱乐设施，操场
    void drawGL_Area(); //Area 西工大长安校区的边界线
    void drawGL_Water();
    void drawGL_Landuse(); //landuse草或森林
    void drawGL_Natural(); //natural 自然区域

    void drawGL_Recover();
    QMap<QString, QPointD> map_Nodes;
    QMap<QString, OpenSMWay> map_Ways;
    QPointD maxPoint; //该变量存储way中highway的x最大值与y最大值
    QPointD minPoint;//该变量存储way中highway的x最小值与y最小值
private:
    QOpenGLFunctions_3_3_Core *core;
};

#endif // OPENSMLOADING_H
