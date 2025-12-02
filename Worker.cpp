#include "Worker.h"
#include <QFile>
#include <QTextStream>
#include <QElapsedTimer>

Worker::Worker(const QString &src, const QString &dst, QObject *parent)
    : QObject(parent), m_src(src), m_dst(dst)
{
}
void Worker::process()
{
    emit logMessage("线程启动！\n");

    QFile in(m_src);
    if (!in.open(QFile::ReadOnly)) {
        emit error("无法打开源文件");
        emit finished();
        return;
    }

    QFile out(m_dst);
    if (!out.open(QFile::WriteOnly | QFile::Truncate)) {
        emit error("无法打开输出文件");
        emit finished();
        return;
    }

    QTextStream ts(&out);
    ts << "unsigned char peData[] = {\n";

    const int block = 4096;
    QByteArray buf;
    buf.resize(block);

    qint64 total = in.size();
    qint64 processed = 0;

    QElapsedTimer speedTimer;
    speedTimer.start();

    // ====== 为保证 UI 显示整齐，独立维护 HEX 格式输出行 ======
    QString hexLineBuffer;
    hexLineBuffer.reserve(4096 * 6);

    int hexLineCount = 0;   // 每 16 字节换行

    emit logMessage("开始按块读取文件...\n");

    while (true) {
        qint64 n = in.read(buf.data(), block);
        if (n <= 0)
            break;

        for (int i = 0; i < n; ++i) {
            unsigned char b = (unsigned char)buf[i];

            // 写入目标 C 数组
            ts << QString("0x%1").arg(b, 2, 16, QLatin1Char('0')).toUpper();
            if (processed + i != total - 1)
                ts << ", ";

            // ===========================
            //   整理日志行（16字节换行）
            // ===========================
            hexLineBuffer += QString("%1 ").arg(b, 2, 16, QLatin1Char('0')).toUpper();
            hexLineCount++;

            if (hexLineCount == 16) {
                hexLineBuffer += "\n";
                emit logMessage(hexLineBuffer);
                hexLineBuffer.clear();
                hexLineCount = 0;
            }
        }

        processed += n;

        // ======================
        //     进度更新（节流）
        // ======================
        qint64 lastProgressUpdate = 0;
        if (processed - lastProgressUpdate > 200 * 1024) { // 每 200KB 更新一次，提高流畅度
            float percent = processed * 100.0 / total;
            emit progress(percent);
            lastProgressUpdate = processed;
        }

        // ======================
        //     速度更新（节流）
        // ======================
        qint64 lastSpeedUpdate = 0;
        if (processed - lastSpeedUpdate > 1024) { // 每 1KB 更新一次
            double sec = speedTimer.elapsed() / 1000.0;
            if (sec > 0) {
                double kbps = (processed / 1024.0) / sec; // KB/s
                emit speed(kbps);
            }
            lastSpeedUpdate = processed;
        }
    }

    // 输出剩余不足 16 字节的数据
    if (!hexLineBuffer.isEmpty()) {
        hexLineBuffer += "\n";
        emit logMessage(hexLineBuffer);
        hexLineBuffer.clear();
    }

    ts << "\n};\n";

    in.close();
    out.close();

    emit progress(100);
    emit speed(0);

    emit logMessage("转换完成！\n线程退出！\n");
    emit finished();
}

