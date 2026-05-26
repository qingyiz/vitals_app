#include "MainWindow.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Vitals"));
    QApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    QApplication::setOrganizationName(QStringLiteral("Vitals"));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/app-icon.svg")));

    Vitals::MainWindow window;
    window.setWindowIcon(QApplication::windowIcon());
    window.setMinimumSize(760, 500);
    window.resize(900, 620);
    window.show();

    return app.exec();
}
