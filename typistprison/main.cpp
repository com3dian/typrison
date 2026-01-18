#include "mainwindow.h"
#include "fontmanager.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QSysInfo>
#include <QFontDatabase>
#include <QDebug>
#include <QScreen>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    qDebug() << "Running on " << QSysInfo::productType();
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Enable translucent background for the application
    app.setAttribute(Qt::AA_EnableHighDpiScaling);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    // Add this line for macOS to enable custom window styling
    // app.setAttribute(Qt::AA_MacWindowToolBar, false);
    
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "typistprison_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    app.setWindowIcon(QIcon(":/icons/typrison_icon.png"));

    MainWindow w;
    w.setWindowTitle("Typrison");

    QString notoSansLightFamily = FontManager::instance().getNotoSansLightFamily();
    QString notoSansDemiLightFamily = FontManager::instance().getNotoSansDemiLightFamily();
    QString notoSansMonoFamily = FontManager::instance().getNotoSansMonoFamily();
    QString notoSansRegularFamily = FontManager::instance().getNotoSansRegularFamily();

    #if defined(Q_OS_WIN)
        qDebug() << "Running on Windows";
        QString styleSheet = QString(
                             "QWidget { font-family: '%1';}"
                             "PlaintextEdit { font-family: '%2'; }"
                             "QMarkdownTextEdit { font-family: '%3'; }"
                             "FictionTextEdit { font-family: '%4'; }"
                             ).arg(notoSansLightFamily)
                              .arg(notoSansMonoFamily)
                              .arg(notoSansLightFamily)
                              .arg(notoSansLightFamily);
    #elif defined(Q_OS_MAC)
        qDebug() << "Running on MacOS";
        QString styleSheet = QString(
                             "QWidget { font-family: '%1';}"
                             "PlaintextEdit { font-family: '%2'; }"
                             "QMarkdownTextEdit { font-family: '%3'; }"
                             "FictionTextEdit { font-family: '%4'; }"
                             ).arg(notoSansLightFamily)
                              .arg(notoSansMonoFamily)
                              .arg(notoSansLightFamily)
                              .arg(notoSansLightFamily);

    #elif defined(Q_OS_LINUX)
        qDebug() << "Running on Linux";
        QString styleSheet = QString(
                             "QWidget { font-family: '%1';}"
                             "PlaintextEdit { font-family: '%2'; }"
                             "QMarkdownTextEdit { font-family: '%3'; }"
                             "FictionTextEdit { font-family: '%4'; }"
                             ).arg(notoSansLightFamily)
                              .arg(notoSansMonoFamily)
                              .arg(notoSansLightFamily)
                              .arg(notoSansLightFamily);
        
    #else
        qDebug() << "Unknown Operating System";
    #endif

    // Set the font as the default application font
    app.setStyleSheet(styleSheet);

    w.setWindowIcon(QIcon(":/icons/typrison_icon.png"));
    w.show();
    return app.exec();
}
