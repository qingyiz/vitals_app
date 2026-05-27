#include "platform/linux/LinuxMemoryCollector.h"

#include <QFile>
#include <QRegularExpression>
#include <QTextStream>
#include <QtGlobal>

namespace Vitals {

namespace {

quint64 parseKilobytes(const QString& line)
{
    static const QRegularExpression valuePattern(QStringLiteral(R"((\d+))"));
    const QRegularExpressionMatch match = valuePattern.match(line);
    if (!match.hasMatch()) {
        return 0;
    }

    return match.captured(1).toULongLong() * 1024ULL;
}

MemorySnapshot readMemInfo()
{
    QFile file(QStringLiteral("/proc/meminfo"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    quint64 totalBytes = 0;
    quint64 availableBytes = 0;
    quint64 freeBytes = 0;

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        if (line.startsWith(QStringLiteral("MemTotal:"))) {
            totalBytes = parseKilobytes(line);
        } else if (line.startsWith(QStringLiteral("MemAvailable:"))) {
            availableBytes = parseKilobytes(line);
        } else if (line.startsWith(QStringLiteral("MemFree:"))) {
            freeBytes = parseKilobytes(line);
        }
    }

    if (availableBytes == 0) {
        availableBytes = freeBytes;
    }

    MemorySnapshot snapshot;
    snapshot.totalBytes = totalBytes;
    snapshot.freeBytes = qMin(availableBytes, totalBytes);
    if (snapshot.totalBytes >= snapshot.freeBytes) {
        snapshot.usedBytes = snapshot.totalBytes - snapshot.freeBytes;
    }
    snapshot.usagePercent = snapshot.totalBytes > 0
        ? static_cast<double>(snapshot.usedBytes) * 100.0 / static_cast<double>(snapshot.totalBytes)
        : 0.0;
    return snapshot;
}

} // namespace

bool LinuxMemoryCollector::initialize()
{
    return readMemInfo().totalBytes > 0;
}

MemorySnapshot LinuxMemoryCollector::collect()
{
    return readMemInfo();
}

} // namespace Vitals
