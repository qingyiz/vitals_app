#pragma once

#include <QString>

namespace Vitals {

class Logger
{
public:
    static void info(const QString& message);
    static void warning(const QString& message);
};

} // namespace Vitals

