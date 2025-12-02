#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextBrowser>
#include <QPushButton>
#include <QString>

#include "Worker.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_openButton_clicked();
    void on_selectButton_clicked();
    void on_convertButton_clicked();

    // Worker -> UI 信号
    void onWorkerProgress(float percent);
    void onWorkerSpeed(double kbps);


private:
    Ui::MainWindow *ui;
    //路径
    QString m_sourceFilePath;
    QString m_destFilePath;

    QString m_logBuffer;
    QTimer *m_logTimer = nullptr;

    double m_maxspeed;
    void log(const QString &msg);
    void visualizeByte(unsigned char b, int index);
};
#endif // MAINWINDOW_H
