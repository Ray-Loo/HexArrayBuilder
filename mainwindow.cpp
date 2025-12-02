#include "mainwindow.h"
#include "ui_mainwindow.h"
// #include "build/Desktop_Qt_6_7_2_MSVC2019_64bit-Debug/ui_mainwindow.h"

#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QThread>
#include <QTimer>

#define logTime 20//刷新时间

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_logTimer = new QTimer(this);
    connect(m_logTimer, &QTimer::timeout, this, [this]() {
        if (!m_logBuffer.isEmpty()) {
            ui->showBrowser->moveCursor(QTextCursor::End);
            ui->showBrowser->insertPlainText(m_logBuffer);
            m_logBuffer.clear();
        }
    });
    m_logTimer->start(logTime);

    // 初始化进度条
    ui->progressBar->setValue(0);
    ui->speedLabel->setText("速度: 0 KB/s");

    ui->speedLabel->hide();
    ui->progressBar->hide();
    m_maxspeed=NULL;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::log(const QString &msg)
{
    ui->showBrowser->append(msg);
}

void MainWindow::visualizeByte(unsigned char b, int index)
{
    QString hex = QString("%1 ").arg(b, 2, 16, QLatin1Char('0')).toUpper();

    ui->showBrowser->moveCursor(QTextCursor::End);
    ui->showBrowser->insertPlainText(hex);

    if (index % 16 == 15)
        ui->showBrowser->insertPlainText("\n");
}

void MainWindow::on_openButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(
        this, "选择源文件", QDir::homePath() + "/Desktop", "所有文件 (*)");

    if (path.isEmpty()) return;

    m_sourceFilePath = path;
    log("选择源文件: " + path);

    ui->speedLabel->hide();
    ui->progressBar->hide();
}

void MainWindow::on_selectButton_clicked()
{
    QString path = QFileDialog::getSaveFileName(
        this, "选择输出文件", QDir::homePath() + "/Desktop", "Header Files (*.h)");

    if (path.isEmpty()) return;

    m_destFilePath = path;
    log("选择输出路径: " + path);

    ui->speedLabel->hide();
    ui->progressBar->hide();
}


// 进度条回调
void MainWindow::onWorkerProgress(float percent)
{
    ui->progressBar->setValue(percent);
}

// 速度回调
void MainWindow::onWorkerSpeed(double kbps)
{
    if(kbps>m_maxspeed){
        m_maxspeed=kbps;
        ui->speedLabel->setText(QString("最高速度: %1 KB/s")
                                    .arg(QString::number(m_maxspeed, 'f', 2)));
    }
}


void MainWindow::on_convertButton_clicked()
{
    if (m_sourceFilePath.isEmpty() || m_destFilePath.isEmpty()) {
        QMessageBox::warning(this, "错误", "请先选择源文件和输出文件路径！");
        return;
    }
    m_maxspeed=0;
    ui->progressBar->setValue(0);
    ui->progressBar->show();
    ui->speedLabel->setText("速度: 0 KB/s");
    ui->speedLabel->show();

    ui->showBrowser->append("开始后台转换任务……");

    QThread *thread = new QThread;
    Worker *worker = new Worker(m_sourceFilePath, m_destFilePath);

    worker->moveToThread(thread);

    // 防止重复连接：先断开所有可能的旧连线
    disconnect(worker, nullptr, this, nullptr);

    // 启动线程
    connect(thread, &QThread::started, worker, &Worker::process);

    // 任务结束 → 退出线程
    connect(worker, &Worker::finished, thread, &QThread::quit);

    // 线程退出 → 删除对象
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    // 收到 worker 的 chunk → UI 缓存，不立即刷新
    connect(worker, &Worker::logMessage, this, [this](const QString &msg) {
        m_logBuffer += msg;
    });

    // 错误消息
    connect(worker, &Worker::error, this, [this](const QString &err) {
        QMessageBox::critical(this, "错误", err);
    });

    // 更新进度
    connect(worker, &Worker::progress, this, &MainWindow::onWorkerProgress);

    // 速度
    connect(worker, &Worker::speed, this, &MainWindow::onWorkerSpeed);

    thread->start();
}
