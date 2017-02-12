#include "mainwindow.h"
#include "ui_mainwindow.h"

AspectRatioPixmapLabel::AspectRatioPixmapLabel(QWidget *parent) :
    QLabel(parent)
{
    this->setMinimumSize(600/3,400/3);
    setScaledContents(false);
}

void AspectRatioPixmapLabel::setPixmap ( const QPixmap & p)
{
    pix = p;
    QLabel::setPixmap(scaledPixmap());
}

int AspectRatioPixmapLabel::heightForWidth( int width ) const
{
    return pix.isNull() ? this->height() : ((qreal)pix.height()*width)/pix.width();
}

QSize AspectRatioPixmapLabel::sizeHint() const
{
    int w = this->width();
    return QSize( w, heightForWidth(w) );
}

QPixmap AspectRatioPixmapLabel::scaledPixmap() const
{
    return pix.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void AspectRatioPixmapLabel::resizeEvent(QResizeEvent * e)
{
    Q_UNUSED(e)

    if(!pix.isNull())
        QLabel::setPixmap(scaledPixmap());
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), m_timer(nullptr), m_ui(new Ui::MainWindow)
{
    QApplication::setApplicationDisplayName(tr("Wink"));
    QApplication::setApplicationName(tr("Wink"));
    QApplication::setApplicationVersion(tr("v1.0.0"));
    QApplication::setOrganizationName(tr("Wink"));
    QApplication::setOrganizationDomain(tr("wink.io"));
    QApplication::setWindowIcon(QIcon("://FullResSquareLogo.png"));

    m_ui->setupUi(this);
    AspectRatioPixmapLabel *label = new AspectRatioPixmapLabel();
    label->setPixmap(QPixmap("://Sxkq4ox.png"));
    m_ui->horizontalLayout_2->insertWidget(0, label);
    delete m_ui->logo;

    QLoggingCategory::setFilterRules(QStringLiteral("qt.network.ssl.warning=false")); // http://stackoverflow.com/questions/26361145/qsslsocket-error-when-ssl-is-not-used

    if(QSslSocket::supportsSsl())
    {
        log(QString("%L1 - SSL Good!").arg(QDateTime::currentDateTime().toString()));
    }
    else
    {
        log(QString("%L1 - SSL Bad!").arg(QDateTime::currentDateTime().toString()));
    }

    connect(m_ui->startButton, &QPushButton::clicked, this, &MainWindow::startTransaction);
    connect(m_ui->cancelButton, &QPushButton::clicked, this, &MainWindow::cancelTransaction);
    connect(m_ui->pay_button, &QPushButton::clicked, this, &MainWindow::payNow);
    cancelTransaction();
    m_ui->pay_button->setIcon(QIcon("://FullResSquareLogo.png"));
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::startup()
{
    QMap<QString, QCameraInfo> map;

    foreach(const QCameraInfo &info, QCameraInfo::availableCameras())
    {
        map.insert(info.description(), info);
    }

    if(!map.isEmpty())
    {
        bool ok;
        QString key = QInputDialog::getItem(this, QString(), tr("Please select a camera:"), map.keys(), 1, false, &ok,
            Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);

        if(ok)
        {
            QCamera *camera = new QCamera(map.value(key), this);
            camera->setViewfinder(m_ui->viewFinder);

            m_imageCapture = new QCameraImageCapture(camera);
            camera->setCaptureMode(QCamera::CaptureStillImage);

            connect(m_imageCapture, &QCameraImageCapture::imageCaptured, this, &MainWindow::imageCaptured);

            camera->start();
            return;
        }
    }
    else
    {
        QMessageBox::critical(this, QString(), tr("No cameras found!"));
    }

    QApplication::quit();
}

void MainWindow::startTransaction()
{
    cancelTransaction();

    m_ui->startButton->setText(tr("Restart Validation"));
    m_ui->cancelButton->setEnabled(true);

    m_timer = new QTimer();
    connect(m_timer, &QTimer::timeout, m_imageCapture, [this] { m_imageCapture->capture(); } );
    m_timer->start(3000);
    m_imageCapture->capture();
}

void MainWindow::cancelTransaction()
{
    if(m_timer)
    {
        delete m_timer;
        m_timer = nullptr;
    }

    m_ui->startButton->setText(tr("Start Validation"));
    m_ui->cancelButton->setEnabled(false);
    m_ui->total_price_edit->clear();
    m_ui->total_price_edit->setDisabled(true);
    m_ui->text_label->setText(tr("Validation Status: <font color='red'>None</font>"));
    m_ui->pay_button->setEnabled(false);
    setStyleSheet("");
}

void MainWindow::imageCaptured(int id, const QImage &preview)
{
    log(QString("%L1 - Frame Number %L2").arg(QDateTime::currentDateTime().toString()).arg(id));

    if(!preview.save(QDir::tempPath() + "/temp.jpg"))
    {
        log(QString("%L1 - Save image failed!").arg(QDateTime::currentDateTime().toString())); return;
    }
    else
    {
        log(QString("%L1 - Save image passed!").arg(QDateTime::currentDateTime().toString()));
    }

    QFile file(QDir::tempPath() + "/temp.jpg");

    if(!file.open(QIODevice::ReadOnly))
    {
        log(QString("%L1 - Open file failed!").arg(QDateTime::currentDateTime().toString())); return;
    }
    else
    {
        log(QString("%L1 - Open file passed!").arg(QDateTime::currentDateTime().toString()));
    }

    QByteArray data = file.readAll();

    if((file.error() != QFile::NoError) || data.isEmpty())
    {
        log(QString("%L1 - Read file failed!").arg(QDateTime::currentDateTime().toString()));  return;
    }
    else
    {
        log(QString("%L1 - Read file passed!").arg(QDateTime::currentDateTime().toString()));
    }

    file.close();

    if(!QFile::remove(QDir::tempPath() + "/temp.jpg"))
    {
        log(QString("%L1 - Remove file failed!").arg(QDateTime::currentDateTime().toString()));  return;
    }
    else
    {
        log(QString("%L1 - Remove file passed!").arg(QDateTime::currentDateTime().toString()));
    }

    QNetworkAccessManager *manager0 = new QNetworkAccessManager(this);

    connect(manager0, &QNetworkAccessManager::finished, this, [this, manager0] (QNetworkReply *reply0) {

        QByteArray data0 = reply0->readAll();
        log(QString("%L1 - Detect reply got %L2").arg(QDateTime::currentDateTime().toString()).arg(QString::fromLatin1(data0)));

        if((reply0->error() == QNetworkReply::NoError) && (!data0.isEmpty()))
        {
            log(QString("%L1 - Detect reply response passed!").arg(QDateTime::currentDateTime().toString()));

            QJsonDocument json0 = QJsonDocument::fromJson(data0);

            if(!json0.isEmpty())
            {
                log(QString("%L1 - Detect reply good json!").arg(QDateTime::currentDateTime().toString()));

                QStringList faceIdsList;

                foreach(const QJsonValue &value, json0.array())
                {
                    QString crap = value.toObject().value("faceId").toString();
                    if(!crap.isEmpty()) faceIdsList.append(value.toObject().value("faceId").toString());
                    log(QString("%L1 - Detect reply faceId - %L2").arg(QDateTime::currentDateTime().toString()).arg(crap));
                }

                if(!faceIdsList.isEmpty())
                {
                    log(QString("%L1 - Detect reply yes faceIds!").arg(QDateTime::currentDateTime().toString()));

                    QNetworkAccessManager *manager1 = new QNetworkAccessManager(this);

                    connect(manager1, &QNetworkAccessManager::finished, this, [this, manager1] (QNetworkReply *reply1) {

                        QByteArray data1 = reply1->readAll();
                        log(QString("%L1 - Identify reply got %L2").arg(QDateTime::currentDateTime().toString()).arg(QString::fromLatin1(data1)));

                        if((reply1->error() == QNetworkReply::NoError) && (!data1.isEmpty()))
                        {
                            log(QString("%L1 - Identify reply response passed!").arg(QDateTime::currentDateTime().toString()));

                            QJsonDocument json1 = QJsonDocument::fromJson(data1);

                            if(!json1.isEmpty())
                            {
                                log(QString("%L1 - Identify reply good json!").arg(QDateTime::currentDateTime().toString()));

                                QStringList candidateList;

                                foreach(const QJsonValue &value, json1.array())
                                {
                                    QString crap = value.toObject().value("candidates").toArray().first().toObject().value("personId").toString();
                                    if(!crap.isEmpty()) candidateList.append(crap);
                                    log(QString("%L1 - Identify reply candidate - %L2").arg(QDateTime::currentDateTime().toString()).arg(crap));
                                }

                                if(!candidateList.isEmpty())
                                {
                                    log(QString("%L1 - Identify reply yes candidateList!").arg(QDateTime::currentDateTime().toString()));

                                    QNetworkAccessManager *manager2 = new QNetworkAccessManager(this);

                                    connect(manager2, &QNetworkAccessManager::finished, this, [this, manager2] (QNetworkReply *reply2) {

                                        QByteArray data2 = reply2->readAll();
                                        log(QString("%L1 - Person reply got %L2").arg(QDateTime::currentDateTime().toString()).arg(QString::fromLatin1(data2)));

                                        if((reply2->error() == QNetworkReply::NoError) && (!data2.isEmpty()))
                                        {
                                            log(QString("%L1 - Person reply response passed!").arg(QDateTime::currentDateTime().toString()));

                                            QJsonDocument json2 = QJsonDocument::fromJson(data2);

                                            if(!json2.isEmpty())
                                            {
                                                log(QString("%L1 - Person reply good json!").arg(QDateTime::currentDateTime().toString()));

                                                m_name = json2.object().value("name").toString();
                                                m_userData = json2.object().value("userData").toString();

                                                log(QString("%L1 - name = %L2 - userData = %L3").arg(QDateTime::currentDateTime().toString()).arg(m_name).arg(m_userData));

                                                if(m_timer)
                                                {
                                                    delete m_timer;
                                                    m_timer = nullptr;
                                                }

                                                m_ui->total_price_edit->setDisabled(false);
                                                m_ui->pay_button->setEnabled(true);
                                                m_ui->text_label->setText(tr("Authentication Status: <font color='blue'>%L1</font>").arg(m_name));
                                                setStyleSheet("background-color:#7FFF7F;");
                                            }
                                            else
                                            {
                                                log(QString("%L1 - Person reply bad json!").arg(QDateTime::currentDateTime().toString()));
                                            }
                                        }
                                        else
                                        {
                                            log(QString("%L1 - Person reply response failed!").arg(QDateTime::currentDateTime().toString()));
                                        }

                                        manager2->deleteLater();
                                        reply2->deleteLater();
                                    });

                                    QNetworkRequest request2 = QNetworkRequest(QUrl("https://westus.api.cognitive.microsoft.com/face/v1.0/persongroups/" MICROSOFT_GROUP "/persons/" + candidateList.first()));
                                    request2.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
                                    request2.setRawHeader("Ocp-Apim-Subscription-Key", MICROSOFT_KEY);
                                    QNetworkReply *reply2 = manager2->get(request2);
                                    log(QString("%L1 - Sending URL %L2").arg(QDateTime::currentDateTime().toString()).arg(request2.url().toString()));

                                    if(!reply2)
                                    {
                                        log(QString("%L1 - Person reply send failed!").arg(QDateTime::currentDateTime().toString())); manager2->deleteLater();
                                    }
                                    else
                                    {
                                        log(QString("%L1 - Person reply send passed!").arg(QDateTime::currentDateTime().toString()));
                                    }
                                }
                                else
                                {
                                    log(QString("%L1 - Identify reply no candidateList!").arg(QDateTime::currentDateTime().toString()));
                                }
                            }
                            else
                            {
                                log(QString("%L1 - Identify reply bad json!").arg(QDateTime::currentDateTime().toString()));
                            }
                        }
                        else
                        {
                            log(QString("%L1 - Identify reply response failed!").arg(QDateTime::currentDateTime().toString()));
                        }

                        manager1->deleteLater();
                        reply1->deleteLater();
                    });

                    QJsonArray faceIds;

                    foreach(const QString &faceId, faceIdsList)
                    {
                        faceIds.append(faceId); break; // do just one...
                    }

                    QJsonObject outObject;
                    outObject.insert("personGroupId", MICROSOFT_GROUP);
                    outObject.insert("faceIds", faceIds);
                    outObject.insert("maxNumOfCandidatesReturned", 1);
                    outObject.insert("confidenceThreshold", 0.5);
                    QByteArray outData = QJsonDocument(outObject).toJson();
                    log(QString("%L1 - Sending JSON %L2").arg(QDateTime::currentDateTime().toString()).arg(QString::fromLatin1(outData)));

                    QNetworkRequest request1 = QNetworkRequest(QUrl("https://westus.api.cognitive.microsoft.com/face/v1.0/identify"));
                    request1.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
                    request1.setRawHeader("Content-Type", "application/json");
                    request1.setRawHeader("Ocp-Apim-Subscription-Key", MICROSOFT_KEY);
                    QNetworkReply *reply1 = manager1->post(request1, outData);

                    if(!reply1)
                    {
                        log(QString("%L1 - Identify reply send failed!").arg(QDateTime::currentDateTime().toString())); manager1->deleteLater();
                    }
                    else
                    {
                        log(QString("%L1 - Identify reply send passed!").arg(QDateTime::currentDateTime().toString()));
                    }
                }
                else
                {
                    log(QString("%L1 - Detect reply no faceIds!").arg(QDateTime::currentDateTime().toString()));
                }
            }
            else
            {
                log(QString("%L1 - Detect reply bad json!").arg(QDateTime::currentDateTime().toString()));
            }
        }
        else
        {
            log(QString("%L1 - Detect reply response failed!").arg(QDateTime::currentDateTime().toString()));
        }

        reply0->deleteLater();
        manager0->deleteLater();
    });

    QNetworkRequest request0 = QNetworkRequest(QUrl("https://westus.api.cognitive.microsoft.com/face/v1.0/detect"));
    request0.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    request0.setRawHeader("Content-Type", "application/octet-stream");
    request0.setRawHeader("Ocp-Apim-Subscription-Key", MICROSOFT_KEY);
    QNetworkReply *reply0 = manager0->post(request0, data);

    if(!reply0)
    {
        log(QString("%L1 - Detect reply send failed!").arg(QDateTime::currentDateTime().toString())); manager0->deleteLater();
    }
    else
    {
        log(QString("%L1 - Detect reply send passed!").arg(QDateTime::currentDateTime().toString()));
    }
}

void MainWindow::showEvent(QShowEvent *event)
{
    QTimer::singleShot(0, this, &MainWindow::startup);
    QMainWindow::showEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
}

void MainWindow::log(const QString &text)
{
    m_ui->console->appendPlainText(text);
    m_ui->console->verticalScrollBar()->setValue(m_ui->console->verticalScrollBar()->maximum());
    qDebug() << text;
}

void MainWindow::payNow()
{
    QRegularExpressionMatch match = QRegularExpression("\\$?(\\d+(\\.\\d+)?)").match(m_ui->total_price_edit->text());

    if(match.captured(1).isEmpty())
    {
        QMessageBox::critical(this, QString(), tr("Please enter a valid dollar amount"));
        return;
    }

    QString amountValue = match.captured(1);
    log(QString("%L1 - Amount %L2").arg(QDateTime::currentDateTime().toString()).arg(amountValue));

    if(!amountValue.contains("."))
    {
        amountValue.append(".00");
    }

    log(QString("%L1 - Changed to %L2").arg(QDateTime::currentDateTime().toString()).arg(amountValue));

    QNetworkAccessManager *manager0 = new QNetworkAccessManager(this);

    connect(manager0, &QNetworkAccessManager::finished, this, [this, manager0] (QNetworkReply *reply0) {

        QByteArray data0 = reply0->readAll();
        log(QString("%L1 - Worldpay charge got %L2").arg(QDateTime::currentDateTime().toString()).arg(QString::fromLatin1(data0)));

        if((reply0->error() == QNetworkReply::NoError) && (!data0.isEmpty()))
        {
            log(QString("%L1 - Worldpay charge response passed!").arg(QDateTime::currentDateTime().toString()));

            QJsonDocument json0 = QJsonDocument::fromJson(data0);

            if(!json0.isEmpty())
            {
                log(QString("%L1 - Worldpay charge good json!").arg(QDateTime::currentDateTime().toString()));

                if(json0.object().value("success").toBool())
                {
                    QMessageBox::information(this, QString(), tr("Payment Done!"));

                    cancelTransaction();
                }
                else
                {
                    QMessageBox::critical(this, QString(), tr("Payment token not valid!"));
                }
            }
            else
            {
                log(QString("%L1 - Worldpay charge bad json!").arg(QDateTime::currentDateTime().toString()));

                QMessageBox::critical(this, QString(), tr("Charge Failed!"));
            }
        }
        else
        {
            log(QString("%L1 - Worldpay charge failed!").arg(QDateTime::currentDateTime().toString()));

            QMessageBox::critical(this, QString(), tr("Charge Failed!"));
        }

        reply0->deleteLater();
        manager0->deleteLater();
    });

    QJsonObject outObject;
    outObject.insert("amount", match.captured(1));
    QJsonObject subObject0;
    subObject0.insert("paymentMethodId", "1");
    subObject0.insert("customerId", m_userData);
    subObject0.insert("paymentType", "CREDIT_CARD");
    outObject.insert("paymentVaultToken", subObject0);
    QJsonObject subObject1;
    subObject1.insert("developerId", "12345678");
    subObject1.insert("version", "1.2");
    outObject.insert("developerApplication", subObject1);

    QByteArray outData = QJsonDocument(outObject).toJson();
    log(QString("%L1 - Sending JSON %L2").arg(QDateTime::currentDateTime().toString()).arg(QString::fromLatin1(outData)));

    QNetworkRequest request0 = QNetworkRequest(QUrl("https://gwapi.demo.securenet.com/api/Payments/Charge"));
    request0.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    request0.setRawHeader("Content-Type", "application/json");
    request0.setRawHeader("Authorization", "Basic ODAwODkyNzpyY09PMEEvS2ZXckM=");
    QNetworkReply *reply0 = manager0->post(request0, outData);

    if(!reply0)
    {
        log(QString("%L1 - Worldpay charge failed!").arg(QDateTime::currentDateTime().toString())); manager0->deleteLater();

        QMessageBox::critical(this, QString(), tr("Network Error!"));
    }
    else
    {
        log(QString("%L1 - Worldpay charge passed!").arg(QDateTime::currentDateTime().toString()));
    }
}
