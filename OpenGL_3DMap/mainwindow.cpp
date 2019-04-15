#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    m_GLWindow = new OpenGLWindow(this);

    ui->horizontalLayout->addWidget(m_GLWindow);
//    ui->horizontalLayout->setStretch(0,1);
//    ui->horizontalLayout->setStretch(1,3);
    ui->mainToolBar->hide();


    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::slot_updateOpenGL);
    timer->start(10);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event){
    m_GLWindow->KeyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event){
    m_GLWindow->KeyReleaseEvent(event);
}

void MainWindow::on_openfile_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("打开文件"),
                                                    "F:",
                                                    tr("地图文件(*osm)"));
    qDebug()<<fileName;
    if(m_GLWindow != nullptr)
    {
        delete m_GLWindow;
        m_GLWindow = new OpenGLWindow(this, fileName);
        ui->horizontalLayout->addWidget(m_GLWindow);
    }
    else
        m_GLWindow = new OpenGLWindow(this, fileName);

}

void MainWindow::on_recovery_triggered()
{
    m_GLWindow->Recovery();
}
