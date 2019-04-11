#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "openglwindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private:
    OpenGLWindow *m_GLWindow;


private:
    Ui::MainWindow *ui;

private slots:
    void slot_updateOpenGL(){
      m_GLWindow->update();
    }
};

#endif // MAINWINDOW_H
