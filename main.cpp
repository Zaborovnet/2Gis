#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include "appcontroller.h"
#include "wordmodel.h"

int main(int argc, char* argv[])
{
  // Настройка приложения
  qRegisterMetaType<QVector<QPair<QString, int>>>("QVector<QPair<QString,int>>");

  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QGuiApplication app(argc, argv);

  QCoreApplication::setOrganizationName("Company");
  QCoreApplication::setApplicationName("WordAnalyzer");

  QQuickStyle::setStyle("Material");

  // Регистрация типов для QML
  qmlRegisterType<WordModel>("WordModel", 1, 0, "WordModel");

  // Создание контроллера
  AppController* controller_po = new AppController();

  // Регистрация в QML
  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("appController", controller_po);
  engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

  // Остановка при завершении
  QObject::connect(&app, &QGuiApplication::aboutToQuit, [controller_po]() {
    controller_po->cancelProcessing();
    delete controller_po;
  });

  return app.exec();
}
