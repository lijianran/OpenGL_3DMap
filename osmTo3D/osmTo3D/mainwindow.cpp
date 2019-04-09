#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScrollArea>
#include <QTimer>


//OpenSMLoading *osm;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QPalette pal(this->palette());

    pal.setColor(QPalette::Background, QColor(99, 103, 106));
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    /************ GL ************/
    oglManager = new OGLManager(this);

    ui->centralWidget->layout()->addWidget(oglManager);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateOGL);
    timer->start(10);

    /************ OSM ************/


    /********** PAINT *************/
    QWidget *widget =new QWidget();
    QScrollArea *s = new QScrollArea();

    s->setGeometry(700, 110, 600, 600);
    widget->show();
//    paint = new Paint(s);
//    paint->findPathQL();
//    s->setWidget(paint);
    s->show();
    //ui->centralWidget->layout()->addWidget(s);



}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event){
    oglManager->handleKeyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event){
    oglManager->handleKeyReleaseEvent(event);
}

void MainWindow::on_pushButton_clicked(){
    oglManager->isLineMode = !oglManager->isLineMode;
}
