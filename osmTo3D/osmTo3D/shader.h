#ifndef SHADER_H
#define SHADER_H

#include <QDebug>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QString>

class Shader {
    friend class ResourceManager;
public:
    Shader(){}
    void compile(const QString& vertexSource, const QString& fragmentSource, const QString& geometrySource = NULL);
    //  Shader(const QString& vertexSourcePath, const QString& fragmentSourcePath);

    ~Shader();


    Shader& use(){
        shaderProgram->bind();
        return *this;
    }

    //还是把设置着色器uniform变量操作写成Shader里的inline成员函数管理，真的方便很多。
    void setFloat(const QString& name, const GLfloat& value){
        GLuint loc = shaderProgram->uniformLocation(name);
        shaderProgram->setUniformValue(loc, value);
    }

    void setInteger(const QString& name, const GLint& value){
        GLuint loc = shaderProgram->uniformLocation(name);
        shaderProgram->setUniformValue(loc, value);
    }

    void setVector2f(const QString& name, const GLfloat& x, const GLfloat& y){
        GLuint loc = shaderProgram->uniformLocation(name);
        shaderProgram->setUniformValue(loc, QVector2D(x, y));
    }

    void setVector2f(const QString& name, const QVector2D& value){
        GLuint loc = shaderProgram->uniformLocation(name);
        shaderProgram->setUniformValue(loc, value);
    }

    void setVector3f(const QString& name, const GLfloat& x, const GLfloat& y, const GLfloat& z){
        GLuint loc = shaderProgram->uniformLocation(name);
        shaderProgram->setUniformValue(loc, QVector3D(x, y, z));
    }

    void setVector3f(const QString& name, const QVector3D& value){
        GLuint loc = shaderProgram->uniformLocation(name);
        shaderProgram->setUniformValue(loc, value);
    }

    void setVector4f(const QString& name, const GLfloat& x, const GLfloat& y, const GLfloat& z, const GLfloat& w){
        GLuint loc = shaderProgram->uniformLocation(name);
        shaderProgram->setUniformValue(loc, QVector4D(x, y, z, w));
    }

    void setVector4f(const QString& name, const QVector4D& value){
        GLuint loc = shaderProgram->uniformLocation(name);
        shaderProgram->setUniformValue(loc, value);
    }

    void setMatrix4f(const QString& name, const QMatrix4x4& value){
        GLuint loc = shaderProgram->uniformLocation(name);
        shaderProgram->setUniformValue(loc, value);
    }

    void setBool(const QString& name, const GLboolean& value){
        GLuint loc = shaderProgram->uniformLocation(name);
        shaderProgram->setUniformValue(loc, value);
    }
private:
    QOpenGLShaderProgram *shaderProgram;//试着让resourcemanager 删除一下

};

#endif // SHADER_H
