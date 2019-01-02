#ifndef WEBAPIPROCESSOR_H
#define WEBAPIPROCESSOR_H

#include <QObject>
#include <QDebug>
#include <QProcess>
#include <QtCore/QCoreApplication>
#include <QtCore/QThread>
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QMutex>
#include <QtCore/QDataStream>
#include <QMap>
#include <QQueue>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QHttpMultiPart>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QEventLoop>

#include "QtCore/qobjectdefs.h"

#include "fcommand.h"
#include "Definitions.h"
#include "ServiceSettings.h"

class WebApiProcessor : public QObject
{
    Q_OBJECT
    public:
        explicit WebApiProcessor(QObject* pParent = NULL);
        explicit WebApiProcessor(Job* pJob, QObject* pParent = NULL);
        ~WebApiProcessor();

    private:
        QNetworkReply *m_reply;
        QNetworkAccessManager *m_NetworkAccessManager;
        QMutex m_mutexUploadReply, m_mutexCapacityReply, m_mutexUploadRequest, m_mutexCapacityRequest;

    public:
        //static QQueue<QMap<int, UploadReply_t>> AssetsQueue;
        //static QQueue<QMap<int, CapacityStatusReply_t>> CapacityStatusQueue;

        QMap<int, UploadReply_t> m_UploadClipMap;
        //QMap<int, CapacityReply_t> m_CapacityReplyMap;
        SiteProperty_t m_SiteProperty;

    private:
        QByteArray buildContent(QString fileName);
        QByteArray buildJsonContent(QMap<int, UploadReply_t> uploadClipMap);

        void onUploadClipReply(QNetworkReply *reply);
        void onUploadStatusReply(QNetworkReply *reply);
        void onCapacityRequestReply(QNetworkReply *reply);
        void onCapacityStatusReply(QNetworkReply *reply);
    private slots:
        //void onUploadClipReply(QNetworkReply *reply);
        //void onUploadStatusReply(QNetworkReply *reply);
        //void onCapacityRequestReply(QNetworkReply *reply);
        //void onCapacityStatusReply(QNetworkReply *reply);
        void onError(QNetworkReply::NetworkError);

    public slots:
        void doTest(QString tt) {
            //qDebug() << this->thread();
            //qDebug() << QCoreApplication.instance()->thread();
            qDebug() << "file: " << tt;
        }

    public:
        bool doUploadClipRequest(QString fileName);
        bool doUploadStatusRequest(int assetId);
        bool doCapacityCountRequest(QMap<int, UploadReply_t> uploadClipMap);
        bool doCapacityStatusRequest(int capacityId);
        void setSiteProperty(Job* pJob) { m_SiteProperty = pJob->SiteProperty; }
};

#endif // WEBAPIPROCESSOR_H
