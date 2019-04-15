#include "loadmapdata.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <iostream>
#include <QVector2D>
#include <QVector3D>

static const float EPSILON=0.0000000001f;
bool Triangulate(QVector<QVector2D> &contour, QVector<QVector2D> &result);
bool Snip(QVector<QVector2D> &contour,int u,int v,int w,int n,int *V);
bool InsideTriangle(float Ax, float Ay,
                    float Bx, float By,
                    float Cx, float Cy,
                    float Px, float Py);
bool isPolygonCounterClocked(QVector<QVector2D> &contour);

LoadMapData::LoadMapData(): core(nullptr)
{
    maxPoint.x = 0;
    maxPoint.y = 0;
    minPoint.x = 360;
    minPoint.y = 360;
}

LoadMapData::~LoadMapData()
{

}

bool LoadMapData::init(const QString &path)
{
    /************* 1. 打开文件 **************/
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<<"LoadMapData ERROR::FILE CAN NOT OPEN!";
        file.close();
        return false;
    }

    QTextStream in(&file);
    QString line;//文件流

    QString lineSearchNodeKey = "node id=";
    QString lineSearchWayKey = "way id=";

    /************* 2. 读取文件 **************/
    while(!in.atEnd())
    {
        line = in.readLine();
        if(line.indexOf(lineSearchNodeKey) >= 0)
        { //寻找结点
            QStringList list = line.split(" ", QString::SkipEmptyParts);
            QString node_id;//节点ID
            // <node id="1876846188" visible="true" version="1" changeset="12814582" timestamp="2012-08-21T22:28:03Z" user="cabineer" uid="690229" lat="34.0301005" lon="108.7635596"/>
            node_id = list[1].mid(4, list[1].size()-5);
            float y = list[8].mid(5, list[8].size()-6).toFloat();//纬度
            float x = list[9].mid(5, list[8].size()-8).toFloat();//经度

            map_Nodes[node_id] = QPointD(x, y);
        }
        else if(line.indexOf(lineSearchWayKey) >= 0)
        {
            QStringList list = line.split(" ", QString::SkipEmptyParts);
            QString way_id = list[1].mid(4, list[1].size()-5);

            OpenSMWay tempWay;
            QString wayline = in.readLine();
            while(wayline.indexOf("/way") < 0)
            {
                if(wayline.indexOf("nd ref=") >= 0)
                {
                    QStringList list = wayline.split(" ", QString::SkipEmptyParts);
                    tempWay.nodesID.push_back(list[1].mid(5, list[1].size()-8));
                }
                else if(wayline.indexOf("tag k=") >= 0)
                {
                    QStringList list = wayline.split(" ", QString::SkipEmptyParts);
                    QString key = list[1].mid(3, list[1].size()-4);
                    QString value = list[2].mid(3, list[2].size()-6);
                    tempWay.kvPairs[key] = value;
                }
                else
                    qDebug() << "This system can not support this osm file, \"way\" format error";

                wayline = in.readLine();
            }
            map_Ways[way_id] = tempWay;
        }
    }
    return true;
}

void LoadMapData::initGL()
{
    core = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();

    if(this->map_Ways.isEmpty())
    {
        qDebug() << "map size is null!";
        return;
    }

    QMap<QString, OpenSMWay>::iterator iter = this->map_Ways.begin();
    while(iter != this->map_Ways.end())
    {
        QVector<QVector3D> new_positions;             // 放入buffer进行绘图的点，已经由垂线生成了新的点
        QVector<QVector3D> ori_positions;             // 一条way的初始节点位置
        QVector<QVector2D> uvs;
        QVector<QVector3D> normals;

        if(iter.value().kvPairs.contains("highway"))
        {   // 当前只处理公路的三维化
            for(int i = 1; i != iter.value().nodesID.size(); ++i)
            { //拿到way的最小与最大xy值
                QPointD temp = this->map_Nodes[iter.value().nodesID[i]];
                if(temp.x < this->minPoint.x)
                    this->minPoint.x = temp.x;
                if(temp.x > this->maxPoint.x)
                    this->maxPoint.x = temp.x;
                if(temp.y < this->minPoint.y)
                    this->minPoint.y = temp.y;
                if(temp.y > this->maxPoint.y)
                    this->maxPoint.y = temp.y;
            }
            /******** 1.1 highway position -- handle data *********/

            QVector<QVector3D> node2_vecs;                // 存储两个相邻节点的矢量差值，比如，9个节点，会有9个矢量差值
            QVector<QVector3D> per_vecs;                  // 通过矢量差值求得的两点连线的垂线

            QPointD first = this->map_Nodes[iter.value().nodesID[0]];
            ori_positions.push_back(QVector3D(first.x, 0, -first.y));

            for(int i = 1; i != iter.value().nodesID.size(); ++i)
            { //存后八个矢量差值
                QPointD current = this->map_Nodes[iter.value().nodesID[i]];
                QPointD last = this->map_Nodes[iter.value().nodesID[i-1]];

                ori_positions.push_back(QVector3D(current.x, 0, -current.y));
                node2_vecs.push_back(QVector3D(current.x-last.x, 0, -(current.y - last.y)));
            }
            node2_vecs.push_front(node2_vecs.first());//存第一个矢量差值

            for(int i = 0; i < node2_vecs.size(); ++i)//由矢量差值，通过叉乘 计算垂直与路的垂线
                per_vecs.push_back(QVector3D::crossProduct(QVector3D(0, 1, 0), node2_vecs[i]).normalized());

            for(int i = node2_vecs.size()-2; i > 0; --i)//矫正垂线
                per_vecs[i] = (per_vecs[i] + per_vecs[i+1]).normalized();

            //通过垂线 计算新增添的点
            for(int i = 0; i < ori_positions.size()-1; ++i)
            {
                QVector3D cur_b = ori_positions[i] - 0.0001f * per_vecs[i];      //current, 当前点 ,b -- bottom, t -- top
                QVector3D cur_add_b = ori_positions[i] + 0.0001f * per_vecs[i];
                QVector3D next_b = ori_positions[i+1] - 0.0001f * per_vecs[i+1];
                QVector3D next_add_b = ori_positions[i+1] + 0.0001f * per_vecs[i+1];

                QVector3D cur_t = QVector3D(cur_b.x(), 1.0f, cur_b.z());
                QVector3D cur_add_t = QVector3D(cur_add_b.x(), 1.0f, cur_add_b.z());
                QVector3D next_t = QVector3D(next_b.x(), 1.0f, next_b.z());
                QVector3D next_add_t = QVector3D(next_add_b.x(), 1.0f, next_add_b.z());
                //上
                new_positions.push_back(cur_t);
                new_positions.push_back(cur_add_t);
                new_positions.push_back(next_add_t);
                new_positions.push_back(next_t);
                //下
                new_positions.push_back(cur_b);
                new_positions.push_back(next_b);
                new_positions.push_back(next_add_b);
                new_positions.push_back(cur_add_b);
                //前
                new_positions.push_back(cur_b);
                new_positions.push_back(cur_add_b);
                new_positions.push_back(cur_add_t);
                new_positions.push_back(cur_t);
                //后
                new_positions.push_back(next_t);
                new_positions.push_back(next_add_t);
                new_positions.push_back(next_add_b);
                new_positions.push_back(next_b);
                //左
                new_positions.push_back(cur_b);
                new_positions.push_back(cur_t);
                new_positions.push_back(next_t);
                new_positions.push_back(next_b);
                //右
                new_positions.push_back(cur_add_b);
                new_positions.push_back(next_add_b);
                new_positions.push_back(next_add_t);
                new_positions.push_back(cur_add_t);
            }

            /******** 1.2 highway uv -- handle data *********/
            for(int i = 0; i < ori_positions.size()-1; ++i)
            {
                uvs.push_back(QVector2D(0, 1));
                uvs.push_back(QVector2D(1, 1));
                uvs.push_back(QVector2D(1, 0));
                uvs.push_back(QVector2D(0, 0));

                uvs.push_back(QVector2D(0, 1));
                uvs.push_back(QVector2D(0, 0));
                uvs.push_back(QVector2D(1, 0));
                uvs.push_back(QVector2D(1, 1));

                uvs.push_back(QVector2D(0, 0));
                uvs.push_back(QVector2D(0, 0));
                uvs.push_back(QVector2D(0, 0));
                uvs.push_back(QVector2D(0, 0));

                uvs.push_back(QVector2D(0, 0));
                uvs.push_back(QVector2D(0, 0));
                uvs.push_back(QVector2D(0, 0));
                uvs.push_back(QVector2D(0, 0));

                uvs.push_back(QVector2D(0, 0));
                uvs.push_back(QVector2D(0, 0));
                uvs.push_back(QVector2D(0, 0));
                uvs.push_back(QVector2D(0, 0));

                uvs.push_back(QVector2D(0, 0));
                uvs.push_back(QVector2D(0, 0));
                uvs.push_back(QVector2D(0, 0));
                uvs.push_back(QVector2D(0, 0));
            }

            /******** 1.3 highway normal -- handle data *********/
            for(int i = 0; i < new_positions.size(); i += 4)
            { //通过叉乘计算 立方体一个面的法线， 即一次 刷新四个点
                QVector3D vec1 = (new_positions[i+1] - new_positions[i]).normalized();
                QVector3D vec2 = (new_positions[i+2] - new_positions[i]).normalized();
                QVector3D per_vec = QVector3D::crossProduct(vec2, vec1).normalized(); //该面的法向量

                for(int j=0; j<4; ++j)
                    normals.push_back(per_vec);
            }
        }
        else if(iter.value().kvPairs.contains("building") ||
                iter.value().kvPairs.contains("amenity") ||
                iter.value().kvPairs.contains("leisure") ||
                iter.value().kvPairs.contains("area") ||
                iter.value().kvPairs.contains("natural") ||
                iter.value().kvPairs.contains("water") ||
                iter.value().kvPairs.contains("landuse")
                )
        {
            /******** 2.1 building position -- handle data *********/
            float height=0.5;
//            if(iter.value().kvPairs.contains("height"))
//            { //设置建筑物的高度，修改osm文件，加一个tag=height，方便
//                QString num = iter.value().kvPairs["height"];
//                height = num.toFloat();
//            }
//            else
//                height = 0.5;

            QVector<QVector2D> vec_judgeCC;//judge polygon counter clock or clock 判断多边形是顺时针还是逆时针
            for(int i = 0; i != iter.value().nodesID.size(); ++i)
            {
                QPointD current = this->map_Nodes[iter.value().nodesID[i]];
                ori_positions.push_back(QVector3D(current.x, 0, -current.y));
                vec_judgeCC.push_back(QVector2D(current.x, -current.y));
            }
            vec_judgeCC.pop_back();

            for(int i = 0; i != iter.value().nodesID.size()-1; ++i)
            {
                QVector3D cur_b = ori_positions[i];      //current, 当前点 ,b -- bottom, t -- top
                QVector3D cur_t = QVector3D(cur_b.x(), height, cur_b.z());
                QVector3D next_b = ori_positions[i+1];
                QVector3D next_t = QVector3D(next_b.x(), height, next_b.z());

                new_positions.push_back(cur_b);
                new_positions.push_back(next_b);
                new_positions.push_back(next_t);
                new_positions.push_back(cur_t);
            }

            /******** 2.2 building uv -- handle data *********/
            for(int i = 0; i != iter.value().nodesID.size()-1; ++i)
            {
                uvs.push_back(QVector2D(0, 0));
                uvs.push_back(QVector2D(1, 0));
                uvs.push_back(QVector2D(1, 1));
                uvs.push_back(QVector2D(0, 1));
            }

            /******** 2.3 building normal -- handle data *********/
            bool isPolygonCC = isPolygonCounterClocked(vec_judgeCC);
            for(int i = 0; i != new_positions.size(); i += 4)
            {
                QVector3D vec1 = (new_positions[i+1] - new_positions[i]).normalized();
                QVector3D vec2 = (new_positions[i+2] - new_positions[i]).normalized();
                QVector3D per_vec = QVector3D::crossProduct(vec1, vec2).normalized(); //该面的法向量

                for(int j=0; j<4; ++j)
                    if(!isPolygonCC)
                        normals.push_back(per_vec);
                    else
                        normals.push_back(-per_vec);
            }


            /******** 增补建筑物上下面 多边形 ***********/
            /* opengl 的polygon绘图方法不支持 凹边形的直接绘制！！！！*/

            QVector<QVector2D> topFace;
            QVector<QVector2D> result;
            for(int i = 0; i != iter.value().nodesID.size()-1; ++i)
                topFace.push_back(QVector2D(ori_positions[i].x(), ori_positions[i].z()));

            Triangulate(topFace, result);
            iter.value().supFaceNum = result.size();
            for(int i = 0; i != result.size(); ++i)
                new_positions.push_back(QVector3D(result[i].x(), height, result[i].y())); //建筑物 顶面， 顶点
            for(int i = 0; i != result.size(); ++i)
                new_positions.push_back(QVector3D(result[i].x(), 0.0f, result[i].y())); //建筑物 底面， 顶点

            for(int i = 0; i != result.size(); ++i)
            {
                uvs.push_back(QVector2D(0, 1));           //建筑物 顶面，因为已经决定纹理样式采用色块了，故 顶面 的纹理坐标均给 （0，1），无伤大雅
                uvs.push_back(QVector2D(0, 1));           //建筑物 底面
            }

            for(int i = 0; i != result.size(); ++i)
                normals.push_back(QVector3D(0, 1, 0));     //建筑物，多边形顶面
            for(int i = 0; i != result.size(); ++i)
                normals.push_back(QVector3D(0, -1, 0));    //建筑物，多边形底面
        }
        else
        {

        }

        if(!new_positions.isEmpty())
        {
            /********  position -- bind buffer data *********/
            core->glGenBuffers(1, &iter.value().positionVBO);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().positionVBO);
            core->glBufferData(GL_ARRAY_BUFFER, new_positions.size()* int(sizeof(QVector3D)), &new_positions[0], GL_STATIC_DRAW);

            /********  uv -- bind buffer data *********/
            core->glGenBuffers(1, &iter.value().uvVBO);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().uvVBO);
            core->glBufferData(GL_ARRAY_BUFFER, uvs.size()* int(sizeof(QVector2D)), &uvs[0], GL_STATIC_DRAW);

            /******** normal -- bind buffer data *********/
            core->glGenBuffers(1, &iter.value().normalVBO);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().normalVBO);
            core->glBufferData(GL_ARRAY_BUFFER, normals.size()* int(sizeof(QVector3D)), &normals[0], GL_STATIC_DRAW);
        }
        ++iter;
    }
}

//画图
void LoadMapData::drawGL_Highway()
{
    QMap<QString, OpenSMWay>::iterator iter = this->map_Ways.begin();
    while(iter != this->map_Ways.end())
    {
        if(iter.value().kvPairs.contains("highway"))
        {
            core->glEnableVertexAttribArray(0);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().positionVBO);
            core->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            core->glEnableVertexAttribArray(1);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().uvVBO);
            core->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

            core->glEnableVertexAttribArray(2);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().normalVBO);
            core->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            core->glDrawArrays(GL_QUADS, 0, (iter.value().nodesID.size()-1)*24);
        }
        ++iter;
    }
}

void LoadMapData::drawGL_Building()
{
        QMap<QString, OpenSMWay>::iterator iter = this->map_Ways.begin();
        while(iter != this->map_Ways.end())
        {
            if(iter.value().kvPairs.contains("building") && !iter.value().isDraw)
            {
                core->glEnableVertexAttribArray(0);
                core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().positionVBO);
                core->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

                core->glEnableVertexAttribArray(1);
                core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().uvVBO);
                core->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

                core->glEnableVertexAttribArray(2);
                core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().normalVBO);
                core->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

                core->glDrawArrays(GL_QUADS, 0, (iter.value().nodesID.size()-1)*4);
                core->glDrawArrays(GL_TRIANGLES, (iter.value().nodesID.size()-1)*4, iter.value().supFaceNum);
                core->glDrawArrays(GL_TRIANGLES, (iter.value().nodesID.size()-1)*4+iter.value().supFaceNum, iter.value().supFaceNum);

                iter.value().isDraw = true;
            }
            ++iter;
        }
}

void LoadMapData::drawGL_Amenity()
{
    QMap<QString, OpenSMWay>::iterator iter = this->map_Ways.begin();
    while(iter != this->map_Ways.end())
    {
        if(iter.value().kvPairs.contains("amenity") && !iter.value().isDraw)
        {
            core->glEnableVertexAttribArray(0);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().positionVBO);
            core->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            core->glEnableVertexAttribArray(1);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().uvVBO);
            core->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

            core->glEnableVertexAttribArray(2);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().normalVBO);
            core->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            core->glDrawArrays(GL_QUADS, 0, (iter.value().nodesID.size()-1)*4);
            core->glDrawArrays(GL_TRIANGLES, (iter.value().nodesID.size()-1)*4, iter.value().supFaceNum);
            core->glDrawArrays(GL_TRIANGLES, (iter.value().nodesID.size()-1)*4+iter.value().supFaceNum, iter.value().supFaceNum);

            iter.value().isDraw = true;
        }
        ++iter;
    }

}

void LoadMapData::drawGL_Leisure()
{
    QMap<QString, OpenSMWay>::iterator iter = this->map_Ways.begin();
    while(iter != this->map_Ways.end())
    {
        if(iter.value().kvPairs.contains("leisure") && !iter.value().isDraw)
        {
            core->glEnableVertexAttribArray(0);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().positionVBO);
            core->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            core->glEnableVertexAttribArray(1);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().uvVBO);
            core->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

            core->glEnableVertexAttribArray(2);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().normalVBO);
            core->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            core->glDrawArrays(GL_QUADS, 0, (iter.value().nodesID.size()-1)*4);
            core->glDrawArrays(GL_TRIANGLES, (iter.value().nodesID.size()-1)*4, iter.value().supFaceNum); //opengl不支持 凹边形的直接绘制，后面再改
            core->glDrawArrays(GL_TRIANGLES, (iter.value().nodesID.size()-1)*4+iter.value().supFaceNum, iter.value().supFaceNum);

            iter.value().isDraw = true;
        }
        ++iter;
    }
}

void LoadMapData::drawGL_Area()
{
    QMap<QString, OpenSMWay>::iterator iter = this->map_Ways.begin();
    while(iter != this->map_Ways.end())
    {
        if(iter.value().kvPairs.contains("area") && !iter.value().isDraw)
        {
            core->glEnableVertexAttribArray(0);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().positionVBO);
            core->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            core->glEnableVertexAttribArray(1);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().uvVBO);
            core->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

            core->glEnableVertexAttribArray(2);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().normalVBO);
            core->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            core->glDrawArrays(GL_QUADS, 0, (iter.value().nodesID.size()-1)*4);
            core->glDrawArrays(GL_TRIANGLES, (iter.value().nodesID.size()-1)*4, iter.value().supFaceNum);
            core->glDrawArrays(GL_TRIANGLES, (iter.value().nodesID.size()-1)*4+iter.value().supFaceNum, iter.value().supFaceNum);

            iter.value().isDraw = true;
        }
        ++iter;
    }
}

void LoadMapData::drawGL_Water(){
    QMap<QString, OpenSMWay>::iterator iter = this->map_Ways.begin();
    while(iter != this->map_Ways.end())
    {
        if(iter.value().kvPairs.contains("water") && !iter.value().isDraw){
            core->glEnableVertexAttribArray(0);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().positionVBO);
            core->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            core->glEnableVertexAttribArray(1);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().uvVBO);
            core->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

            core->glEnableVertexAttribArray(2);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().normalVBO);
            core->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            core->glDrawArrays(GL_QUADS, 0, (iter.value().nodesID.size()-1)*4);
            core->glDrawArrays(GL_TRIANGLES, (iter.value().nodesID.size()-1)*4, iter.value().supFaceNum);
            core->glDrawArrays(GL_TRIANGLES, (iter.value().nodesID.size()-1)*4+iter.value().supFaceNum, iter.value().supFaceNum);

            iter.value().isDraw = true;
        }
        ++iter;
    }
}

void LoadMapData::drawGL_Landuse()
{
    QMap<QString, OpenSMWay>::iterator iter = this->map_Ways.begin();
    while(iter != this->map_Ways.end())
    {
        if(iter.value().kvPairs.contains("landuse") && !iter.value().isDraw)
        {
            core->glEnableVertexAttribArray(0);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().positionVBO);
            core->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            core->glEnableVertexAttribArray(1);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().uvVBO);
            core->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

            core->glEnableVertexAttribArray(2);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().normalVBO);
            core->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            core->glDrawArrays(GL_QUADS, 0, (iter.value().nodesID.size()-1)*4);
            core->glDrawArrays(GL_TRIANGLES, (iter.value().nodesID.size()-1)*4, iter.value().supFaceNum);
            core->glDrawArrays(GL_TRIANGLES, (iter.value().nodesID.size()-1)*4+iter.value().supFaceNum, iter.value().supFaceNum);

            iter.value().isDraw = true;
        }
        ++iter;
    }
}

void LoadMapData::drawGL_Natural()
{
    QMap<QString, OpenSMWay>::iterator iter = this->map_Ways.begin();
    while(iter != this->map_Ways.end())
    {
        if(iter.value().kvPairs.contains("natural") && !iter.value().isDraw)
        {
            core->glEnableVertexAttribArray(0);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().positionVBO);
            core->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            core->glEnableVertexAttribArray(1);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().uvVBO);
            core->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

            core->glEnableVertexAttribArray(2);
            core->glBindBuffer(GL_ARRAY_BUFFER, iter.value().normalVBO);
            core->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            core->glDrawArrays(GL_QUADS, 0, (iter.value().nodesID.size()-1)*4);
            core->glDrawArrays(GL_TRIANGLES, (iter.value().nodesID.size()-1)*4, iter.value().supFaceNum); //opengl不支持 凹边形的直接绘制，后面再改
            core->glDrawArrays(GL_TRIANGLES, (iter.value().nodesID.size()-1)*4+iter.value().supFaceNum, iter.value().supFaceNum);

            iter.value().isDraw = true;
        }
        ++iter;
    }
}

void LoadMapData::drawGL_Recover()
{
    QMap<QString, OpenSMWay>::iterator iter = this->map_Ways.begin();
    while(iter != this->map_Ways.end())
    {
        iter.value().isDraw = false;
        ++iter;
    }
}


/************* 凹边形的三角剖分法 外边的代码  *****************/

bool Triangulate(QVector<QVector2D> &contour, QVector<QVector2D> &result)
{
    /* allocate and initialize list of Vertices in polygon */

    int n = contour.size();
    if ( n < 3 ) return false;

    int *V = new int[n];

    /* we want a counter-clockwise polygon in V */

    if (isPolygonCounterClocked(contour) )
        for (int v=0; v<n; v++) V[v] = v;
    else
        for(int v=0; v<n; v++) V[v] = (n-1)-v;

    int nv = n;

    /*  remove nv-2 Vertices, creating 1 triangle every time */
    int count = 2*nv;   /* error detection */

    for(int m=0, v=nv-1; nv>2; )
    {
        /* if we loop, it is probably a non-simple polygon */
        if (0 >= (count--))
        {
            //** Triangulate: ERROR - probable bad polygon!

            return false;
        }

        /* three consecutive vertices in current polygon, <u,v,w> */
        int u = v  ; if (nv <= u) u = 0;     /* previous */
        v = u+1; if (nv <= v) v = 0;     /* new v    */
        int w = v+1; if (nv <= w) w = 0;     /* next     */

        if ( Snip(contour,u,v,w,nv,V) )
        {
            int a,b,c,s,t;

            /* true names of the vertices */
            a = V[u]; b = V[v]; c = V[w];

            /* output Triangle */
            result.push_back( contour[a] );
            result.push_back( contour[b] );
            result.push_back( contour[c] );

            m++;

            /* remove v from remaining polygon */
            for(s=v,t=v+1;t<nv;s++,t++)
                V[s] = V[t];
            nv--;

            /* resest error detection counter */
            count = 2*nv;
        }
    }

    delete[] V;
    return true;
}

bool Snip(QVector<QVector2D> &contour,int u,int v,int w,int n,int *V)
{
    int p;
    float Ax, Ay, Bx, By, Cx, Cy, Px, Py;

    Ax = contour[V[u]].x();
    Ay = contour[V[u]].y();

    Bx = contour[V[v]].x();
    By = contour[V[v]].y();

    Cx = contour[V[w]].x();
    Cy = contour[V[w]].y();

    if ( EPSILON > (((Bx-Ax)*(Cy-Ay)) - ((By-Ay)*(Cx-Ax))) )
        return false;

    for (p=0;p<n;p++)
    {
        if( (p == u) || (p == v) || (p == w) )
            continue;
        Px = contour[V[p]].x();
        Py = contour[V[p]].y();
        if (InsideTriangle(Ax,Ay,Bx,By,Cx,Cy,Px,Py))
            return false;
    }

    return true;
}

bool InsideTriangle(float Ax, float Ay,
                    float Bx, float By,
                    float Cx, float Cy,
                    float Px, float Py)
{
    float ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
    float cCROSSap, bCROSScp, aCROSSbp;

    ax = Cx - Bx;  ay = Cy - By;
    bx = Ax - Cx;  by = Ay - Cy;
    cx = Bx - Ax;  cy = By - Ay;
    apx= Px - Ax;  apy= Py - Ay;
    bpx= Px - Bx;  bpy= Py - By;
    cpx= Px - Cx;  cpy= Py - Cy;

    aCROSSbp = ax*bpy - ay*bpx;
    cCROSSap = cx*apy - cy*apx;
    bCROSScp = bx*cpy - by*cpx;

    return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
}

//判断传进来的多边形数组是否是 逆时针
bool isPolygonCounterClocked(QVector<QVector2D> &contour)
{
    if(contour.size() < 3)
    {
        qDebug() << "contour.size is smaller than 3, it can not form a polygon";
        return false;
    }
    /*
    判断组成多边形(无论凹凸性)的点的顺序 是否为逆时针：
    1，找到x最大或y最大的点v1，因为该点必为凸点。
    2，找到两个矢量，v1-v0,与v2-v1, 叉乘这两个矢量，得到的值若为正值，则为逆时针，负为顺时针
    */
    int maxIndex = -1;
    float max = -1.0f;
    for(int i = 0; i < contour.size(); ++i)
        if(contour[i].x() > max)
        { //找最大x的点，
            max = contour[i].x();
            maxIndex = i;
        }

    QVector2D v1, v2;
    if(maxIndex == 0)
    {
        v1 = (contour[0] - contour[contour.size()-1]).normalized();
        v2 = (contour[1] - contour[0]).normalized();
    }
    else if(maxIndex == contour.size()-1)
    {
        v1 = (contour[contour.size()-1] - contour[contour.size()-2]).normalized();
        v2 = (contour[0] - contour[contour.size()-1]).normalized();
    }
    else
    {
        v1 = (contour[maxIndex] - contour[maxIndex-1]).normalized();
        v2 = (contour[maxIndex+1] - contour[maxIndex]).normalized();
    }

    float res = v1.x()*v2.y()-v2.x()*v1.y();

    if(res > 0)
        return true;

    return false;
}
