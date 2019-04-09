#include "light.h"
#include <QDebug>
#include <QVector>
#include <QVector2D>
#include <QVector3D>

Light::Light(): lightVBO(0){
    core = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
}

Light::~Light(){
    if(lightVBO != 0)
        core->glDeleteBuffers(1, &lightVBO);
}

void Light::init(){

    float lightVertices[] = {
        // positions          // normals           // texture coords
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

    core->glGenBuffers(1, &lightVBO);

    core->glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    core->glBufferData(GL_ARRAY_BUFFER, sizeof(lightVertices), lightVertices, GL_STATIC_DRAW);

}

void Light::drawLight(){
    core->glEnableVertexAttribArray(0);
    core->glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    core->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    core->glDrawArrays(GL_TRIANGLES, 0, 36);
}


/**************            COORDINATE             ************************/

Coordinate::Coordinate(): VBO(0){
    core = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
}

Coordinate::~Coordinate(){
    if(VBO != 0)
        core->glDeleteBuffers(1, &VBO);
}

void Coordinate::init(){

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

void Coordinate::draw(){
    core->glEnableVertexAttribArray(0);
    core->glBindBuffer(GL_ARRAY_BUFFER, VBO);
    core->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    core->glDrawArrays(GL_LINES, 0, 6);
}

/**************            IMAGE            ************************/


Image::Image(): positionVBO(0){
    core = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
}

Image::~Image(){
    if(positionVBO != 0)
        core->glDeleteBuffers(1, &positionVBO);
}

void Image::init(){
    QVector<QVector3D> positions;
    positions.push_back(QVector3D(-0.5f, 0.0f, -0.5f));
    positions.push_back(QVector3D(0.5f, 0.0f, -0.5f));
    positions.push_back(QVector3D(0.5f, 0.0f, 0.5f));
    positions.push_back(QVector3D(-0.5f, 0.0f, 0.5f));

    core->glGenBuffers(1, &positionVBO);
    core->glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    core->glBufferData(GL_ARRAY_BUFFER, positions.size()* sizeof(QVector3D), &positions[0], GL_STATIC_DRAW);

    QVector<QVector2D> uvs;
    uvs.push_back(QVector2D(0, 0));
    uvs.push_back(QVector2D(1, 0));
    uvs.push_back(QVector2D(1, 1));
    uvs.push_back(QVector2D(0, 1));

    core->glGenBuffers(1, &uvVBO);
    core->glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
    core->glBufferData(GL_ARRAY_BUFFER, uvs.size()* sizeof(QVector2D), &uvs[0], GL_STATIC_DRAW);
}

void Image::draw(){
    core->glEnableVertexAttribArray(0);
    core->glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    core->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    core->glEnableVertexAttribArray(1);
    core->glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
    core->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    core->glDrawArrays(GL_QUADS, 0, 4);
}
