#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <map>
#include <QString>
#include <QDebug>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

class Shader {
    friend class ResourceManager;
public:
    Shader(){}
    ~Shader(){}

    void compile(const QString& vertexSource, const QString& fragmentSource, const QString& geometrySource = nullptr);

    Shader& use()
    {
        m_ShaderProgram->bind();
        return *this;
    }

    void setFloat(const QString& name, const GLfloat& value)
    {
        m_ShaderProgram->setUniformValue(m_ShaderProgram->uniformLocation(name), value);
    }

    void setInteger(const QString& name, const GLint& value)
    {
        m_ShaderProgram->setUniformValue(m_ShaderProgram->uniformLocation(name), value);
    }

    void setVector2f(const QString& name, const GLfloat& x, const GLfloat& y)
    {
        m_ShaderProgram->setUniformValue(m_ShaderProgram->uniformLocation(name), QVector2D(x, y));
    }

    void setVector2f(const QString& name, const QVector2D& value)
    {
        m_ShaderProgram->setUniformValue(m_ShaderProgram->uniformLocation(name), value);
    }

    void setVector3f(const QString& name, const GLfloat& x, const GLfloat& y, const GLfloat& z)
    {
        m_ShaderProgram->setUniformValue(m_ShaderProgram->uniformLocation(name), QVector3D(x, y, z));
    }

    void setVector3f(const QString& name, const QVector3D& value)
    {
        m_ShaderProgram->setUniformValue(m_ShaderProgram->uniformLocation(name), value);
    }

    void setVector4f(const QString& name, const GLfloat& x, const GLfloat& y, const GLfloat& z, const GLfloat& w)
    {
        m_ShaderProgram->setUniformValue(m_ShaderProgram->uniformLocation(name), QVector4D(x, y, z, w));
    }

    void setVector4f(const QString& name, const QVector4D& value)
    {
        m_ShaderProgram->setUniformValue(m_ShaderProgram->uniformLocation(name), value);
    }

    void setMatrix4f(const QString& name, const QMatrix4x4& value)
    {
        m_ShaderProgram->setUniformValue(m_ShaderProgram->uniformLocation(name), value);
    }

    void setBool(const QString& name, const GLboolean& value)
    {
        m_ShaderProgram->setUniformValue(m_ShaderProgram->uniformLocation(name), value);
    }

private:
    QOpenGLShaderProgram *m_ShaderProgram;

};

class Texture2D
{
    friend class ResourceManager;
public:
    Texture2D();
    ~Texture2D(){}
    void generate(const QString& file);
    void bind() const;

    QOpenGLTexture::TextureFormat internal_format;
    QOpenGLTexture::WrapMode wrap_s;
    QOpenGLTexture::WrapMode wrap_t;
    QOpenGLTexture::Filter filter_min;
    QOpenGLTexture::Filter filter_max;

private:
    QOpenGLTexture *m_texture;
};

class ResourceManager
{
public:
    static std::map<QString, Shader> map_Shaders;
    static std::map<QString, Texture2D> map_Textures;
    static Shader loadShader(const QString& name, const QString& vShaderFile, const QString& fShaderFile, const QString& gShaderfile = nullptr);
    static Shader getShader(const QString&  name);
    static Texture2D loadTexture(const QString&  name, const QString& file, GLboolean alpha = false);
    static Texture2D getTexture(const QString&  name);
    static void clear();
private:
    ResourceManager(){}
};

#endif // RESOURCEMANAGER_H
