#include "carcontrol.h"

const GLfloat CARSPEED = 0.1f;
const GLfloat CARYAW = 0.0f;
const GLfloat CARTURNSPEED = 8.0f;

CarControl::CarControl(): osm(nullptr), model(nullptr),
 front(QVector3D(0.0f, 0.0f, -1.0f)), movementSpeed(CARSPEED),
 turnSpeed(CARTURNSPEED), yaw(CARYAW){

}

CarControl::~CarControl(){
  if(model)
    delete model;
}

void CarControl::setCarModel(const QString &path){
  model = new Model();
  if(!model->init(path))
    qDebug() << "ERROR::CARCONTROL--SETCARMODEL::PATH ERROR!";
}

void CarControl::setOSM(OpenSMLoading *osm){
    this->osm = osm;
}

void CarControl::setCarOriginMatrix(QVector3D trans, QVector3D scale, GLfloat rotAngle, QVector3D rotAxis){
  QMatrix4x4 model;
  model.rotate(rotAngle, rotAxis);
  model.scale(scale);
  model.translate(trans);

  this->originModel = model;
}

QMatrix4x4 CarControl::getModelMatrix(){
  QMatrix4x4 model;
  model.translate(position.x(), position.y()+0.01f, position.z());
  model.rotate(yaw, QVector3D(0, 1, 0));

  return model * this->originModel;
}

void CarControl::draw(GLboolean isOpenLighting){
  model->draw(isOpenLighting);
}

void CarControl::processKeyboard(Car_Movement direction, GLfloat deltaTime){
  GLfloat velocity = this->movementSpeed * deltaTime;
  GLfloat turnVelocity = this->turnSpeed * deltaTime;

  if (direction == CAR_STRAIGHT)
    this->position += this->front * velocity;
  if (direction == CAR_BACKWARD)
    this->position -= this->front * velocity;
  if (direction == CAR_LEFT){
    this->yaw += turnVelocity;
    GLfloat yawR = qDegreesToRadians(this->yaw);
    QVector3D front3(-sin(yawR), 0.0f, -cos(yawR));
    this->front = front3.normalized();
  }
  if (direction == CAR_RIGHT){
    this->yaw -= turnVelocity;
    GLfloat yawR = qDegreesToRadians(this->yaw);
    QVector3D front3(-sin(yawR), 0.0f, -cos(yawR));
    this->front = front3.normalized();
  }
}
