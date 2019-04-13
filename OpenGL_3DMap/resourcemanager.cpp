#include "resourcemanager.h"

//shader
void Shader::compile(const QString& vertexSource, const QString& fragmentSource, const QString& geometrySource)
{
    QOpenGLShader vertexShader(QOpenGLShader::Vertex);
    bool success = vertexShader.compileSourceFile(vertexSource);
    if(!success)
    {
        qDebug() << "ERROR::SHADER::VERTEX::COMPILATION_FAILED" << endl;
        qDebug() << vertexShader.log() << endl;
    }

    QOpenGLShader fragmentShader(QOpenGLShader::Fragment);
    success  = fragmentShader.compileSourceFile(fragmentSource);
    if(!success)
    {
        qDebug() << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED" << endl;
        qDebug() << fragmentShader.log() << endl;
    }

    QOpenGLShader geometryShader(QOpenGLShader::Geometry);
    if(geometrySource != nullptr)
    {
        success  = geometryShader.compileSourceFile(geometrySource);
        if(!success)
        {
            qDebug() << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED" << endl;
            qDebug() << geometryShader.log() << endl;
        }
    }


    m_ShaderProgram = new QOpenGLShaderProgram();
    m_ShaderProgram->addShader(&vertexShader);
    m_ShaderProgram->addShader(&fragmentShader);
    if(geometrySource != nullptr)
        m_ShaderProgram->addShader(&geometryShader);
    success = m_ShaderProgram->link();
    if(!success)
    {
        qDebug() << "ERROR::SHADER::PROGRAM::LINKING_FAILED" << endl;
        qDebug() << m_ShaderProgram->log() << endl;
    }
}

//texture2d
Texture2D::Texture2D():internal_format(QOpenGLTexture::RGBFormat),
    wrap_s(QOpenGLTexture::Repeat),
    wrap_t(QOpenGLTexture::Repeat),
    filter_min(QOpenGLTexture::Linear),
    filter_max(QOpenGLTexture::Linear),
    m_texture(nullptr)
{

}

void Texture2D::generate(const QString &file)
{
    m_texture = new QOpenGLTexture(QOpenGLTexture::Target2D); //直接生成绑定一个2d纹理, 并生成多级纹理MipMaps
    m_texture->setFormat(internal_format);
    m_texture->setData(QImage(file).mirrored(), QOpenGLTexture::GenerateMipMaps);

    m_texture->setWrapMode(QOpenGLTexture::DirectionS, wrap_s);// 等于glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    m_texture->setWrapMode(QOpenGLTexture::DirectionT, wrap_t);//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    m_texture->setMinificationFilter(filter_min);   //等价于glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_texture->setMagnificationFilter(filter_max);  //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture2D::bind() const
{
    m_texture->bind();
}


//manager
std::map<QString, Shader> ResourceManager::map_Shaders;
std::map<QString, Texture2D> ResourceManager::map_Textures;

Shader ResourceManager::loadShader(const QString& name, const QString& vShaderFile, const QString& fShaderFile, const QString& gShaderfile)
{
    Shader shader;
    shader.compile(vShaderFile, fShaderFile, gShaderfile == nullptr ? nullptr : gShaderfile);
    map_Shaders[name] = shader;
    return map_Shaders[name];
}

Shader ResourceManager::getShader(const QString& name)
{
    return map_Shaders[name];
}

Texture2D ResourceManager::loadTexture(const QString&  name, const QString& file, GLboolean alpha)
{
    Texture2D texture;

    if(alpha)
    {
        texture.internal_format = QOpenGLTexture::RGBAFormat;
        texture.wrap_s = QOpenGLTexture::ClampToBorder;
        texture.wrap_t = QOpenGLTexture::ClampToBorder;
    }

    texture.generate(file);
    map_Textures[name] = texture;
    return texture;
}

Texture2D ResourceManager::getTexture(const QString& name)
{
    return map_Textures[name];
}

void ResourceManager::clear()
{
    for (std::map<QString, Shader>::iterator iter = map_Shaders.begin(); iter != map_Shaders.end(); ++iter)
        delete iter->second.m_ShaderProgram;
    for (std::map<QString, Texture2D>::iterator iter = map_Textures.begin(); iter != map_Textures.end(); ++iter)
        delete iter->second.m_texture;
}
