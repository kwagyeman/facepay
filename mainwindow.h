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
#define WORLDPAY_KEY "93836c33-2ad2-4598-bd41-b91e3a19e81d"

namespace Ui
{
    class MainWindow;
}

class AspectRatioPixmapLabel : public QLabel
{
    Q_OBJECT
public:
    explicit AspectRatioPixmapLabel(QWidget *parent = 0);
    virtual ~AspectRatioPixmapLabel() {}
    virtual int heightForWidth( int width ) const;
    virtual QSize sizeHint() const;
    QPixmap scaledPixmap() const;
public slots:
    void setPixmap ( const QPixmap & );
    void resizeEvent(QResizeEvent *);
private:
    QPixmap pix;
};

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
    void resizeEvent(QResizeEvent *event);

private:

    void log(const QString &text);

    QCameraImageCapture *m_imageCapture;
    QTimer *m_timer;
    QString m_name;
    QString m_userData;
    Ui::MainWindow *m_ui;
};

#endif // MAINWINDOW_H
