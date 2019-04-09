#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "paint.h"
#include "oglmanager.h"
#include "opensmloading.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:
    void keyPressEvent(QKeyEvent *event);   //键盘按下事件
    void keyReleaseEvent(QKeyEvent *event);  //键盘释放事件
private:
    Ui::MainWindow *ui;
    Paint *paint;
    OGLManager *oglManager;

private slots:
    void updateOGL(){
      oglManager->update();
    }
    void on_pushButton_clicked();
};

#endif // MAINWINDOW_H
