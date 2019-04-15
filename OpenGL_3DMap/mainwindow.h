#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QFileDialog>
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

    Ui::MainWindow *ui;

private slots:
    void slot_updateOpenGL()
    {
      m_GLWindow->update();
    }
    void on_openfile_triggered();
    void on_recovery_triggered();
};

#endif // MAINWINDOW_H
