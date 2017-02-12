#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtNetwork>
#include <QtMultimedia>
#include <QtMultimediaWidgets>

#define MICROSOFT_KEY "e5bdbaf52816430bb3ff0ef29850855f"
#define MICROSOFT_GROUP "facepay"
#define WOLRPAY_KEY "xxx"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:

    void startup();
    void startTransaction();
    void cancelTransaction();
    void imageCaptured(int id, const QImage &preview);
    void payNow();

protected:

    void showEvent(QShowEvent *event);

private:

    void log(const QString &text);

    QCameraImageCapture *m_imageCapture;
    QTimer *m_timer;
    QString m_name;
    QString m_userData;
    Ui::MainWindow *m_ui;
};

#endif // MAINWINDOW_H
