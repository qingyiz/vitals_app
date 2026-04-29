#pragma once

class QObject;

namespace Vitals {

class TaskbarIndicator;

TaskbarIndicator* createTaskbarIndicator(QObject* parent = nullptr);

} // namespace Vitals

