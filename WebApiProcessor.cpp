#include "WebApiProcessor.h"
WebApiProcessor::WebApiProcessor(QObject* pParent/* = NULL*/ )
    : QObject(pParent)
{
    qDebug() << "WebApiProcessor SiteProperty cameras: " << m_SiteProperty.cameras.size();
}

WebApiProcessor::WebApiProcessor(Job* pJob, QObject* pParent/* = NULL*/ )
    : QObject(pParent),
      m_SiteProperty(pJob->SiteProperty)
{
    qDebug() << "WebApiProcessor SiteProperty cameras: " << m_SiteProperty.cameras.size();
}

WebApiProcessor::~WebApiProcessor()
{
    //delete m_pJob;
}

/*
*/
bool WebApiProcessor::doUploadClipRequest(QString fileName)
{
    try {
        //m_mutexUploadRequest.lock();
        qDebug() << "Upload: " << fileName;

        QNetworkAccessManager *nam = new QNetworkAccessManager();

        QEventLoop loop;
        QObject::connect(nam, &QNetworkAccessManager::finished, &loop, [=](QNetworkReply* reply){
            onUploadClipReply(reply);
            //loop.quit();
        }, Qt::QueuedConnection);
/*
        QObject::connect(nam, &QNetworkAccessManager::finished, [=](QNetworkReply* reply){
            onUploadClipReply(reply);
            //reply->deleteLater();
            return true;
        });
 */
        qDebug() << "Host: " << QUrl("https://" + m_SiteProperty.urlApi + ":443/assets");
        QNetworkRequest request(QUrl("https://" + m_SiteProperty.urlApi + ":443/assets")); //our server with php-script

        QSslConfiguration config = request.sslConfiguration();
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        config.setProtocol(QSsl::TlsV1_2);

        //m_NetworkAccessManager.connectToHostEncrypted(QString("https://" + m_SiteProperty.urlApi + ":443/assets"), 443, config);

        request.setSslConfiguration(config);

        QString bound="margin"; //name of the boundary

        QByteArray data = buildContent(fileName);
        request.setRawHeader(QByteArray("Content-Type"), QByteArray(QString("multipart/form-data; boundary=" + bound).toLatin1()));
        request.setRawHeader(QByteArray("IVA-CustomerKey"), m_SiteProperty.customerApiKey.toLatin1());
        request.setRawHeader(QByteArray("Content-Length"), QByteArray(QString::number(data.length()).toLatin1()));

        this->m_reply = nam->post(request,data);

        qDebug() << data.data();

        //connect(am, SIGNAL(finished(QNetworkReply*)), this, SLOT(onUploadClipReply(QNetworkReply*)));
        connect(this->m_reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(onError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
        //while (thread()->eventDispatcher()->hasPendingEvents())
         //   thread()->eventDispatcher()->processEvents(QEventLoop::ExcludeUserInputEvents);
        loop.exec();
        //loop.processEvents(QEventLoop::AllEvents, 1500);
        //m_mutexUploadRequest.unlock();
        return true;
    }
    catch (...)
    {
        return false;
    }
}

/*
*/
bool WebApiProcessor::doCapacityCountRequest(QMap<int, UploadReply_t> uploadClipMap)
{
    try {
        //m_mutexCapacityRequest.lock();
        QNetworkAccessManager *nam = new QNetworkAccessManager();
        QEventLoop loop;
        connect(nam, &QNetworkAccessManager::finished, &loop, [=](QNetworkReply* reply){
            onCapacityRequestReply(reply);
        });

        QNetworkRequest request(QUrl(QString("https://" + m_SiteProperty.urlApi + ":443/capacity/request")));

        QSslConfiguration config = request.sslConfiguration();
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        config.setProtocol(QSsl::TlsV1_2);

        request.setSslConfiguration(config);

        QString bound="margin"; //name of the boundary

        QByteArray data = this->buildJsonContent(uploadClipMap);
        //qDebug() << "Json Capacity: " << data;
        request.setRawHeader(QByteArray("Content-Type"), QByteArray(QString("application/x-www-form-urlencoded; boundary=" + bound).toLatin1()));
        request.setRawHeader(QByteArray("IVA-CustomerKey"), m_SiteProperty.customerApiKey.toLatin1());
        request.setRawHeader(QByteArray("Content-Length"), QByteArray(QString::number(data.length()).toLatin1()));

        this->m_reply = nam->post(request, data);

        qDebug() << "TEST doCapacityCountRequest";
        qDebug() << data.data();
        //connect(am, SIGNAL(finished(QNetworkReply*)), this, SLOT(onCapacityRequestReply(QNetworkReply*)));
        connect(this->m_reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(onError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
        loop.exec();
        //m_mutexCapacityRequest.unlock();
        return true;
    }
    catch (...)
    {
        return false;
    }
}

/*
*/
bool WebApiProcessor::doUploadStatusRequest(int assetId)
{
    try {
        QNetworkAccessManager *nam = new QNetworkAccessManager();

        QEventLoop loop;
        connect(nam, &QNetworkAccessManager::finished, &loop, [=](QNetworkReply* reply){
            onUploadStatusReply(reply);
            //reply->deleteLater();
            //loop.exit(0);
        }, Qt::QueuedConnection);



        QNetworkRequest request(QUrl(QString("https://" + m_SiteProperty.urlApi + ":443/assets/").append(QString::number(assetId)))); //our server with php-script

        QSslConfiguration config = request.sslConfiguration();
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        config.setProtocol(QSsl::TlsV1_2);
        request.setSslConfiguration(config);

        request.setRawHeader(QByteArray("Accept"), QByteArray("application/json"));
        request.setRawHeader(QByteArray("Content-Type"), QByteArray("application/json"));
        request.setRawHeader(QByteArray("IVA-CustomerKey"), m_SiteProperty.customerApiKey.toLatin1());
        //request.setRawHeader(QByteArray("Content-Length"), QByteArray(QString::number(data.length()).toLatin1()));

        this->m_reply = nam->get(request);

        //connect(am, SIGNAL(finished(QNetworkReply*)), this, SLOT(onUploadStatusReply(QNetworkReply*)));
        connect(this->m_reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(onError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
        //this->m_reply->deleteLater();
        loop.exec();
        //while (thread()->eventDispatcher()->hasPendingEvents())
        //    thread()->eventDispatcher()->processEvents(QEventLoop::ExcludeUserInputEvents);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

/*
*/
bool WebApiProcessor::doCapacityStatusRequest(int capacityId)
{
    try {
        QNetworkAccessManager *nam = new QNetworkAccessManager(this);
        QEventLoop loop;
        connect(nam, &QNetworkAccessManager::finished, &loop, [=](QNetworkReply* reply){
            onCapacityStatusReply(reply);
            //reply->deleteLater();
        });

        QNetworkRequest request(QUrl(QString("https://" + m_SiteProperty.urlApi + ":443/capacity/status/").append(QString::number(capacityId)))); //our server with php-script

        QSslConfiguration config = request.sslConfiguration();
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        config.setProtocol(QSsl::TlsV1_2);
        request.setSslConfiguration(config);

        request.setRawHeader(QByteArray("Accept"), QByteArray("application/json"));
        request.setRawHeader(QByteArray("Content-Type"), QByteArray("application/json"));
        request.setRawHeader(QByteArray("IVA-CustomerKey"), m_SiteProperty.customerApiKey.toLatin1());
        //request.setRawHeader(QByteArray("Content-Length"), QByteArray(QString::number(data.length()).toLatin1()));

        this->m_reply = nam->get(request);

        //connect(am, SIGNAL(finished(QNetworkReply*)), this, SLOT(onCapacityStatusReply(QNetworkReply*)));
        connect(this->m_reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(onError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
        loop.exec();
        return true;
    }
    catch (...)
    {
        return false;
    }
}

QByteArray WebApiProcessor::buildJsonContent(QMap<int, UploadReply_t> uploadClipMap)
{
    QJsonObject capacityJson;
    QJsonArray assetIDs, cameraIDs, weights;

    capacityJson["SiteID"] = m_SiteProperty.siteID;
    foreach (const int &key, uploadClipMap.keys())
    {
        CameraProperty_t camProp = m_SiteProperty.cameras[key];
        assetIDs.append(QJsonValue(uploadClipMap[key].AssetID));
        cameraIDs.append(QJsonValue(camProp.channelID));
        weights.append(QJsonValue(camProp.weight));
    }
    capacityJson["AssetIDs"] = assetIDs;
    capacityJson["CameraIDs"] = cameraIDs;
    capacityJson["Weights"] = weights;

    return QJsonDocument(capacityJson).toJson();
}

/*
*/
QByteArray WebApiProcessor::buildContent(QString fileName)
{
    QString bound="margin"; //name of the boundary
    QByteArray data;
    //according to rfc 1867 we need to put this string here:
    QString strContent = "--" + bound + "\r\n";
    strContent.append("Content-Disposition: form-data; name=\"action\"\r\n\r\n");
    strContent.append("assets\r\n"); //our API name.
    strContent.append("--" + bound + "\r\n"); //according to rfc 1867
    strContent.append("Content-Disposition: form-data; name=\"asset\"; filename=\""
                      + fileName.split('/').last()
                      + "\"\r\n"); //name of the input is "asset" in form, next one is a file name.
    strContent.append("Content-Type: video/mp4\r\n\r\n"); //data type
    //qDebug() << strContent;
    data.append(strContent);
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        data.clear();
        return data;
    }
    data.append(file.readAll()); //read the file
    strContent.append("\r\n");
    strContent.append("--" + bound + "--\r\n"); //closing boundary according to rfc 1867
    return data.append(strContent);
}

/*
*/
void WebApiProcessor::onUploadClipReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        //m_mutexUploadReply.lock();
        qDebug() << "TEST onUploadClipReply";
        QByteArray arr = reply->readAll();
        qDebug() << arr.data();
        QJsonDocument json = QJsonDocument::fromJson(arr);

        UploadReply_t ur;
        ur.AssetID = json["AssetID"] == QJsonValue::Undefined ? -1 : json["AssetID"].toInt();
        ur.Status = json["Status"] == QJsonValue::Undefined ? -1 : json["Status"].toInt();
        ur.ErrMessage = json["ErrMessage"] == QJsonValue::Undefined ? "" : json["ErrMessage"].toString();

        qDebug() << "onUploadClipReply()" << m_UploadClipMap.size() << " | " << m_SiteProperty.cameras.size();
        //collect assets for capacity request
        if(/*ur.Status == "success" &&*/ m_UploadClipMap.size() < m_SiteProperty.cameras.size())
        {
            m_UploadClipMap.insert(m_UploadClipMap.size(), ur);
            qDebug() << "onUploadClipReply()" << m_UploadClipMap.size() << " || " << m_SiteProperty.cameras.size();
            if(m_UploadClipMap.size() == m_SiteProperty.cameras.size())
            {
                AssetsQueue.enqueue(m_UploadClipMap);
            }
        }

        reply->deleteLater();

        if(ur.AssetID != -1) doUploadStatusRequest(ur.AssetID);
        //m_mutexUploadReply.unlock();
    }
    else
    {
        qDebug() << reply->errorString();
    }
}

/*
*/
void WebApiProcessor::onUploadStatusReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        //m_mutexUploadReply.lock();
        qDebug() << "TEST onUploadStatusReply";
        QByteArray arr = reply->readAll();
        qDebug() << arr.data();
        QJsonDocument json = QJsonDocument::fromJson(arr);

        UploadStatus_t us;
        us.videoID = json["videoID"] == QJsonValue::Undefined ? -1 : json["videoID"].toInt();
        us.assetStatus = json["assetStatus"] == QJsonValue::Undefined ? -1 : json["assetStatus"].toInt();
        us.ErrMessage = json["errMessage"] == QJsonValue::Undefined ? "" : json["errMessage"].toString();
        //WebApiProcessor::m_listUploadStatusReply.append(us);
        qDebug() << "onUploadStatusReply()" << m_UploadClipMap.size() << " ||| " << AssetsQueue.size() << " ||| " << m_SiteProperty.cameras.size();

        //make sure no upload errors
        if (us.videoID != -1)
        {
            if (AssetsQueue.size() > 0 && AssetsQueue.head().size() == m_SiteProperty.cameras.size())
            {
                qDebug() << "all upload requests processed.";
                QMap<int, UploadReply_t> uploadClipMap = AssetsQueue.dequeue();
                //qDebug() << "onUploadStatusReply() VideoID=" << QString::number(us.videoID);
                doCapacityCountRequest(uploadClipMap);
                m_UploadClipMap.clear();
            }
        }

        reply->deleteLater();
        //m_mutexUploadReply.unlock();
    }
    else
    {
        qDebug() << reply->errorString();
    }
}

/*
*/
void WebApiProcessor::onCapacityRequestReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        //m_mutexCapacityReply.lock();
        qDebug() << "TEST onCapacityRequestReply";
        QByteArray arr = reply->readAll();
        qDebug() << arr.data();
        QJsonDocument json = QJsonDocument::fromJson(arr);

        CapacityReply_t cr;
        //cr.RequestStatus = "requested";
        cr.CapacityID = json["CapacityID"] == QJsonValue::Undefined ? -1 : json["CapacityID"].toInt();
        cr.Status = json["Status"] == QJsonValue::Undefined ? -1 : json["Status"].toInt();
        cr.ErrMessage = json["ErrMessage"] == QJsonValue::Undefined ? "" : json["ErrMessage"].toString();
        if(cr.ErrMessage == "success")
        {
            //m_CapacityReplyMap.append(m_CapacityReplyMap.size(), cr);
            doCapacityStatusRequest(cr.CapacityID);
        }
        //collect assets for capacity request

        reply->deleteLater();
        //m_mutexCapacityReply.unlock();
    }
    else
    {
        qDebug() << reply->errorString();
    }
}

/*
*/
void WebApiProcessor::onCapacityStatusReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        //m_mutexCapacityReply.lock();
        qDebug() << "TEST onCapacityStatusReply";
        QByteArray arr = reply->readAll();
        qDebug() << arr.data();
        QJsonDocument json = QJsonDocument::fromJson(arr);

        //CapacityReply_t cr = m_CapacityReplyMap.first();
        QUrl url = reply->request().url();

        CapacityStatusReply_t csr;
        //cr.RequestStatus = "processed";
        csr.CapacityID = url.toString().split('/').last().toInt();// cr.CapacityID;
        QJsonArray cidArr = json["CameraIDs"] == QJsonValue::Undefined ? QJsonArray() : json["CameraIDs"].toArray();
        for (int i = 0; i < cidArr.size(); i++)
           csr.CameraIDs[i] = cidArr.at(i).toInt();
        QJsonArray cntArr = json["Counts"] == QJsonValue::Undefined ? QJsonArray() : json["Counts"].toArray();
        for (int i = 0; i < cntArr.size(); i++)
           csr.Counts[i] = cntArr.at(i).toInt();
        csr.Total = json["Total"] == QJsonValue::Undefined ? -1 : json["Total"].toInt();

        QMap<int, CapacityStatusReply_t> csrMap;
        csrMap.insert(CapacityStatusQueue.size(), csr);
        CapacityStatusQueue.enqueue(csrMap);

        reply->deleteLater();
        //m_mutexCapacityReply.unlock();
    }
    else
    {
        qDebug() << reply->errorString();
    }
}

void WebApiProcessor::onError(QNetworkReply::NetworkError err)
{
    qDebug() << "Error#: " << err;
}
