#include "paint.h"
#include "oglmanager.h"
#include <QTime>
#include <QDebug>

OpenSMLoading osm_paint;
QMap<QString, QPointF> map_revPoints;//存储矫正后的所有的node的新位置,用于画线
QVector<VertexNode> v_vertexGraph; //QLearning 的图结构的 Result表
QVector<QVector<float>> vv_QLabel;


//QRgb BACK_COLOR = qRgb(255, 255, 255);
QRgb BACK_COLOR = qRgb(210, 210, 210);

int PAINT_WIDTH = 2000;
int PAINT_HEIGHT = 1500;

float R(int state, int action);
float getQMax(int state, int action);
void QLearning();

Paint::Paint(QWidget *parent) : QWidget(parent){
    this->setGeometry(0, 12, PAINT_WIDTH, PAINT_HEIGHT);

    image = QImage(PAINT_WIDTH, PAINT_HEIGHT, QImage::Format_RGB32);
    backColor = BACK_COLOR;
    image.fill(backColor);

    /*********** 1 osm 画点 ************/

    osm_paint.init("C:/Users/lijianran/Desktop/2.osm");
    if(osm_paint.map_Nodes.isEmpty()){
        qDebug() << "osm nodes empty!!!";
    }

    QVector<QPointF> vec_drawpoints;
    QPointD offsetPoint;//该点存放，该map中最小的x坐标与最大的y坐标的值

    QMap<QString, QPointD>::const_iterator i = osm_paint.map_Nodes.constBegin();
    QMap<QString ,QPointD>::iterator first = osm_paint.map_Nodes.begin();
    while(i != osm_paint.map_Nodes.constEnd()){
        QPointD temp;
        temp.x = i.value().x - first.value().x;
        temp.y = i.value().y - first.value().y;

        if(offsetPoint.x > temp.x)
            offsetPoint.x = temp.x;
        if(offsetPoint.y > temp.y)
            offsetPoint.y = temp.y;
        ++i;
    }//第一次 找到oriPoint,即整个图形要偏移的位置距离

    offsetPoint.x *= 30000;//西工大 osm， 20000
    offsetPoint.y *= 30000;
    i = osm_paint.map_Nodes.constBegin();
    while(i != osm_paint.map_Nodes.constEnd()){
        QPointD temp;
        temp.x = (i.value().x - first.value().x) * 30000;
        temp.y = (i.value().y - first.value().y) * 30000;

        if(offsetPoint.x < 0)
            temp.x -= offsetPoint.x;
        if(offsetPoint.y < 0)
            temp.y = PAINT_HEIGHT - (temp.y - offsetPoint.y);

        vec_drawpoints.push_back(QPointF(temp.x, temp.y));
        map_revPoints[i.key()] = QPointF(temp.x, temp.y);
        ++i;
    }
    drawPoints(this->image, vec_drawpoints); //暂时先不画 一会还原

    /*********** 2 osm 画线 ************/
    QMap<QString, OpenSMWay>::const_iterator iter = osm_paint.map_Ways.constBegin();
    while(iter != osm_paint.map_Ways.constEnd()){
        if(iter.value().kvPairs.contains("highway")){
            QPen pen;
            pen.setColor(Qt::black);
            pen.setWidth(1);

            QVector<QPointF> vec_drawlines;
            for(int i = 0; i != iter.value().nodesID.size(); ++i){
                vec_drawlines.push_back(map_revPoints[iter.value().nodesID[i]]);
            }

            drawLines(this->image, vec_drawlines); //暂时先不画 一会还原
        }
        ++iter;
    }

}

void Paint::paintEvent(QPaintEvent *event){
    QPainter painter(this);
    painter.drawImage(0, 0, image);
}

void Paint::mouseMoveEvent(QMouseEvent *event){
    if(event->buttons()&Qt::LeftButton){

    }
}

void Paint::mousePressEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton){
        //    points.push_back(event->pos());
        //    drawPoints(this->image, points);
    }
}

void Paint::mouseReleaseEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton){

    }
}

void Paint::drawPoints(QImage &theImage, QVector<QPointF> &points, QPen *pen){
    QPainter painter(&theImage);

    if(!pen){
        QPen pen;
        painter.setPen(pen);
    }else
        painter.setPen(*pen);

    if(!points.isEmpty()){
        painter.drawPoints(&points[0], points.size());
        update();
    }
}

void Paint::drawLines(QImage &theImage, QVector<QPointF> &points, QPen *pen){
    QPainter painter(&theImage);
    if(!pen){
        QPen pen;
        painter.setPen(pen);
    }else
        painter.setPen(*pen);

    for(int i = 0; i < points.size()-1; ++i)
        painter.drawLine(points[i], points[i+1]);

    update();
}

void Paint::findPathQL(){
    /*************  统计map_ways 中共用node的个数及共用次数***************/
    QVector<QString> vStr_highwayNode;   //拿到way中关于highway的所有节点id，肯定会有一些结点重复
    QVector<QString> vStr_publicNode;   //拿到highway的公共点
    QMap<QString, OpenSMWay>::iterator it = osm_paint.map_Ways.begin();
    while(it != osm_paint.map_Ways.end()){
        if(it.value().kvPairs.contains("highway")){
            for(int i = 0; i != it.value().nodesID.size(); ++i)
                vStr_highwayNode.push_back(it.value().nodesID[i]);
        }
        ++it;
    }

    QMap<QString, QPointD>::iterator it1 = osm_paint.map_Nodes.begin();
    while(it1 != osm_paint.map_Nodes.end()){
        int count = 0;
        for(int i = 0; i < vStr_highwayNode.size(); ++i){
            if(it1.key() ==  vStr_highwayNode[i])
                count++;
        }
        if(count > 1){
            vStr_publicNode.push_back(it1.key());
            //      qDebug() << it1.key() << " count: " << count;
        }
        ++it1;
    }
    qDebug() << " public node count: "<< vStr_publicNode.size();
    qDebug() << " highway all node count: " << vStr_highwayNode.size() << " all way node size: " << osm_paint.map_Nodes.size();

    /*
   * 画出highway所有的公共节点
   */
    //  QVector<QPointF> v_drawpoints;
    //  for(int i = 0; i < vStr_publicNode.size(); ++i){
    //    v_drawpoints.push_back(map_revPoints[vStr_publicNode[i]]);
    //    qDebug() << vStr_publicNode[i];
    //  }
    //  QPen *pen;
    //  pen = new QPen();
    //  pen->setColor(Qt::red);
    //  pen->setWidth(5);
    //  this->drawPoints(this->image, v_drawpoints, pen);

    /************* 把公共node和 他们与下一个公共node的关系 存入graph结构  ***************/
    /*
   *存储每条way中拥有的公共顶点
  */
    QVector<QVector<QString>> vv_pubSinwayNode;    //vv_public single way node 64
    QMap<QString, OpenSMWay>::const_iterator iter = osm_paint.map_Ways.constBegin();
    while(iter != osm_paint.map_Ways.constEnd()){
        if(iter.value().kvPairs.contains("highway")){
            QVector<QString> temp; //存储一条way所拥有的公共node
            for(int i = 0; i != iter.value().nodesID.size(); ++i){
                if(vStr_publicNode.contains(iter.value().nodesID[i]))
                    temp.push_back(iter.value().nodesID[i]);
            }
            //      qDebug() << "way id: " << iter.key() << " way node size: " << iter.value().nodesID.size();
            //      qDebug() << "this way public node num: " << temp.size();
            //      for(int i = 0; i < temp.size(); ++i){
            //        qDebug() << temp[i];
            //      }
            vv_pubSinwayNode.push_back(temp);
        }
        ++iter;
    }

    /*
   * 构造graph，找每个共用vertex的邻边
  */
    //  QVector<VertexNode> v_vertexGraph;
    for(int i = 0; i < vStr_publicNode.size(); ++i){ //64个共用顶点
        VertexNode vertex;
        vertex.id = vStr_publicNode[i];
        for(int j = 0; j < vv_pubSinwayNode.size(); ++j){
            int idIndex = vv_pubSinwayNode[j].indexOf(vertex.id);      //如果某way的 公共节点vec里 含有这个公共节点
            if(idIndex == -1 || vv_pubSinwayNode[j].size() == 1) //如果该数组没有这个公共节点，或者该条路只有一个公共顶点 则跳过，判断下一个数组
                continue;
            if(idIndex == 0){ //该公共节点 为这个公共节点数组的第一个点
                vertex.insertEdge(vv_pubSinwayNode[j][idIndex+1]);
            }else if(idIndex == vv_pubSinwayNode[j].size()-1){ //该公共节点 为这个公共节点数组的最后一个点
                vertex.insertEdge(vv_pubSinwayNode[j][idIndex-1]);
            }else{
                vertex.insertEdge(vv_pubSinwayNode[j][idIndex-1]);
                vertex.insertEdge(vv_pubSinwayNode[j][idIndex+1]);
            }

        }
        v_vertexGraph.push_back(vertex);
    }
    /*
   * 打印vertex图结构
   */
    //    qDebug() << "vertex graph size: " << v_vertexGraph.size();
    //    for(int i = 0; i < v_vertexGraph.size(); ++i){
    //      qDebug() << i << " vertex id :" << v_vertexGraph[i].id;
    //      EdgeNode *temp = v_vertexGraph[i].firstedge;
    //      while(temp){
    //        qDebug() << "  edge id: " << temp->id;
    //        temp = temp->next;
    //      }
    //    }


    /*
   * 统计graph 每个vertex边的总数
   */
    for(int i = 0; i < v_vertexGraph.size(); ++i){
        int edgeCount = 0;        //统计每个vertex的边的数量
        EdgeNode *temp = v_vertexGraph[i].firstedge;
        while(temp){
            edgeCount++;
            temp = temp->next;
        }
        v_vertexGraph[i].edgeCount = edgeCount;
        //   qDebug() << i << " " << v_vertexGraph[i].id << edgeCount;
    }

    /**************** 生成result表 ******************/
    int startIndex = 2, endIndex = 44; //选择共用顶点的起点与终点。
    for(int i = 0; i < v_vertexGraph.size(); ++i){
        EdgeNode *temp = v_vertexGraph[i].firstedge;
        while(temp){
            if(temp->id == vStr_publicNode[endIndex]){
                temp->weight = 100.0f;           //给重点附近的 共用顶点赋权值 为100.0f
            }
            temp = temp->next;
        }
    }

    /**************** 生成Q表 ******************/
    for(int i = 0;i < v_vertexGraph.size(); ++i){  //初始化Q表
        QVector<float> temp;
        for(int j = 0; j < v_vertexGraph[i].edgeCount; ++j)
            temp.push_back(0.0f);
        vv_QLabel.push_back(temp);
    }

    /*
  * 训练
  */
    //  QTime time1;
    //  time1.start();
    //  qDebug() << v_vertexGraph[8].id;
    for(int i = 0; i < 50000; ++i){
        //qDebug() << i << " " << vv_QLabel[8][0] << " " << vv_QLabel[8][1] << " " << vv_QLabel[8][2] << " " << vv_QLabel[8][3];
        QLearning();
    }
    //  qDebug() << time1.elapsed();

    /*
   * 输出 vv_QLabel
  */
    //  qDebug() << "        state    0   1   2   3";
    //  for(int i = 0 ; i < vv_QLabel.size(); ++i){
    //    if(vv_QLabel[i].size() == 2)
    //      qDebug() << i << " " << v_vertexGraph[i].id << vv_QLabel[i][0] << " " << vv_QLabel[i][1];
    //    else if(vv_QLabel[i].size() == 3)
    //      qDebug() << i << " " << v_vertexGraph[i].id << vv_QLabel[i][0] << " " << vv_QLabel[i][1] << " " << vv_QLabel[i][2];
    //    else if(vv_QLabel[i].size() == 4)
    //      qDebug() << i << " " << v_vertexGraph[i].id << vv_QLabel[i][0] << " " << vv_QLabel[i][1] << " " << vv_QLabel[i][2] << " " << vv_QLabel[i][3];
    //    else
    //      qDebug() << "error";
    //  }

    /*
   * 输出路径
  */
    QVector<QPointF> sevec;
    sevec.push_back(map_revPoints[v_vertexGraph[startIndex].id]);
    sevec.push_back(map_revPoints[v_vertexGraph[endIndex].id]);
    QPen *pen;
    pen = new QPen();
    pen->setWidth(8);
    drawPoints(this->image, sevec, pen);

    QVector<QPointF> wayValues;
    QString key = v_vertexGraph[startIndex].id;
    int tempIndex = startIndex;
    while(1){
        wayValues.push_back(map_revPoints[key]);
        if(key == vStr_publicNode[endIndex])
            break;
        int maxAct = 0;
        int maxValue = vv_QLabel[tempIndex][0];
        for(int i = 1; i < vv_QLabel[tempIndex].size(); ++i)
            if(maxValue < vv_QLabel[tempIndex][i]){
                maxValue = vv_QLabel[tempIndex][i];
                maxAct = i;
            }
        EdgeNode *edge = v_vertexGraph[tempIndex].firstedge;
        for(int i = 0; i < maxAct; ++i)
            edge = edge->next;
        //    qDebug() << key << " "<< maxAct;
        key = edge->id;
        for(int i = 0; i < vStr_publicNode.size();++i)
            if(vStr_publicNode[i] == edge->id)
                tempIndex = i;
    }

    QPen *pen1;
    pen1 = new QPen();
    pen1->setColor(Qt::red);
    pen1->setWidth(5);
    this->drawPoints(this->image, wayValues, pen1);
}

float R(int state, int action){
    EdgeNode *temp = v_vertexGraph[state].firstedge;
    for(int i = 0; i < action; ++i)
        temp = temp->next;

    return temp->weight;
}

float getQMax(int state, int action){ //得到state状态下的最大收益
    EdgeNode *temp = v_vertexGraph[state].firstedge;
    for(int i = 0; i < action; ++i)
        temp = temp->next;

    int s_next; //state 在action下的 新state
    for(int i = 0; i < v_vertexGraph.size(); ++i)
        if(v_vertexGraph[i].id == temp->id)
            s_next = i;

    float max = vv_QLabel[s_next][0];
    for(int i = 1; i < vv_QLabel[s_next].size(); ++i){
        if(max < vv_QLabel[s_next][i])
            max = vv_QLabel[s_next][i];
    }

    return max;
}

void QLearning(){
    int s = qrand()%vv_QLabel.size();
    int a = qrand()%vv_QLabel[s].size();
    float alpha = 0.1f;
    float gamma = 0.8f;

    vv_QLabel[s][a] = (1-alpha)*vv_QLabel[s][a] + alpha * (R(s, a) + gamma * getQMax(s, a));
}
