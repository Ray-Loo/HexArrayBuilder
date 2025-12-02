#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QString>

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(const QString &src, const QString &dst, QObject *parent = nullptr);

signals:
    void logMessage(const QString &msg);
    void finished();
    void error(const QString &err);
    void progress(float percent);     // 0~100
    void speed(double mbps);         // MB/s

public slots:
    void process();   // 执行任务

private:
    QString m_src;
    QString m_dst;
};

#endif // WORKER_H
