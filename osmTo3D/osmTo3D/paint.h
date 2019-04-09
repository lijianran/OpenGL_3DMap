#ifndef PAINT_H
#define PAINT_H

#include <QWidget>
#include<QPainter>
#include<QMouseEvent>
#include <QVector>
#include <QImage>
#include "opensmloading.h"

class Paint : public QWidget{
public:
  explicit Paint(QWidget *parent = nullptr);

  void findPathQL();
protected:
  void paintEvent(QPaintEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
private:
  void drawPoints(QImage &theImage, QVector<QPointF> &points, QPen *pen = nullptr);
  void drawLines(QImage &theImage, QVector<QPointF> &points, QPen *pen = nullptr);
  QRgb backColor;
  QImage image;

  /******* OSM 相关参数*******/
};

/************ QL 相关参数 图结构 ************/
struct EdgeNode{
  EdgeNode():weight(0.0f), next(nullptr){}
  EdgeNode(QString id, float weight = 0.0f):next(nullptr){
    this->id = id;
    this->weight = weight;
  }

  QString id;//存储的是 下一个公共节点的id，不再考虑way的id
  float weight;
  EdgeNode *next;
};

struct VertexNode{
  VertexNode():firstedge(nullptr), lastedge(nullptr), edgeCount(0){}
  QString id;// 公共节点的id
  EdgeNode *firstedge;

  void insertEdge(QString id, float weight = 0.0f){
    if(firstedge){
      lastedge->next = new EdgeNode(id, weight);
      lastedge = lastedge->next;
    }else{
      firstedge = new EdgeNode(id, weight);
      lastedge = firstedge;
    }
  }

  EdgeNode *lastedge;
  int edgeCount;
};

#endif // PAINT_H
