#include <QtCore/QCoreApplication>
#include <QDebug>

#include "ContactEvents.h"


int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

    ServiceSettings* pServiceSettings = new ServiceSettings(QThread::currentThread());

    AssetsQueue.clear();
    /**/
    ContactEvents* eventHandler = new ContactEvents(pServiceSettings, QThread::currentThread());

    eventHandler->handleContactEvents();

/*
    //XFetcher* wap  = new XFetcher(pServiceSettings->GetJobFromConfig(), QThread::currentThread());;
    //xf.doUploadClipRequest(QString("D:\\X-VideoClips\\clip_1543965754_BerlinDemo_0.mp4"));
    WebApiProcessor* wap = new WebApiProcessor(pServiceSettings->GetJobFromConfig(), QThread::currentThread());
    wap->doUploadClipRequest(QString(pServiceSettings->GetJobFromConfig()->videoFolder + "/clip_1545086098_BerlinDemo_0.mp4"));
    wap->doUploadClipRequest(QString(pServiceSettings->GetJobFromConfig()->videoFolder + "/clip_1545086098_BerlinDemo_1.mp4"));
    wap->doUploadClipRequest(QString(pServiceSettings->GetJobFromConfig()->videoFolder + "/clip_1545086098_BerlinDemo_2.mp4"));
    wap->doUploadClipRequest(QString(pServiceSettings->GetJobFromConfig()->videoFolder + "/clip_1545086098_BerlinDemo_3.mp4"));

    WebApiProcessor *wap = new WebApiProcessor(pServiceSettings->GetJobFromConfig(), a.instance());
      QFuture<void> uploadClipJob = QtConcurrent::run(wap, &WebApiProcessor::doUploadClipRequest, QString(pServiceSettings->GetJobFromConfig()->videoFolder + "/clip_1545086098_BerlinDemo_0.mp4"));
      //uploadClipJob.waitForFinished();
      uploadClipJob = QtConcurrent::run(wap, &WebApiProcessor::doUploadClipRequest, QString(pServiceSettings->GetJobFromConfig()->videoFolder + "/clip_1545086098_BerlinDemo_1.mp4"));
      //uploadClipJob.waitForFinished();
      uploadClipJob = QtConcurrent::run(wap, &WebApiProcessor::doUploadClipRequest, QString(pServiceSettings->GetJobFromConfig()->videoFolder + "/clip_1545086098_BerlinDemo_2.mp4"));
      //uploadClipJob.waitForFinished();
      uploadClipJob = QtConcurrent::run(wap, &WebApiProcessor::doUploadClipRequest, QString(pServiceSettings->GetJobFromConfig()->videoFolder + "/clip_1545086098_BerlinDemo_3.mp4"));
 */

    return a.exec();
}
