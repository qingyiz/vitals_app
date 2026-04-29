#include "log/Logger.h"

#include <QDebug>

namespace Vitals {

void Logger::info(const QString& message)
{
    qInfo().noquote() << message;
}

void Logger::warning(const QString& message)
{
    qWarning().noquote() << message;
}

} // namespace Vitals

