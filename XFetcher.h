#pragma once

#include <QObject>
#include <QDebug>
#include <QProcess>
#include <QtCore/QCoreApplication>
//#include <QApplication>
#include <QtCore/QThread>
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QMutex>
#include <QtCore/QDataStream>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QHttpMultiPart>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QScopedPointer>
#include <QFuture>
#include <QtConcurrent/QtConcurrentRun>
#include <QList>

#include "QtCore/qobjectdefs.h"

#include "fcommand.h"
#include "Definitions.h"
#include "RecorderAccess.h"
#include "../XmlRpcLinux/XmlRpcValue.h"
#include "ServiceSettings.h"
#include "WebApiProcessor.h"

class XFetcher : public QObject
{
	Q_OBJECT

public:
	explicit XFetcher(QObject* pParent = NULL);
    explicit XFetcher(Job* pJob, QObject* pParent = NULL);
	~XFetcher();

private:
	//! Data type of index entry for I-frame index list
	typedef struct
	{
		qint64  timeStamp;                              //!< Time stamp of I-frame
		qint64  filePos;                                //!< Position of I-frame in file
		quint32 cameraPattern;                          //!< Bit pattern for camera
	} IndexEntry_t;

public:
    WebApiProcessor m_WebApiProcessor;

private:
    //ServiceSettings* m_ServiceSettings;
	QMutex mutexJobScheduler;                          //!< Mutex for job scheduler
    QMutex m_mutexFFMPEG;
	volatile bool m_isStopThreadWanted;                //!< True to stop thread
	volatile bool m_isSuspendThreadWanted;             //!< True to suspend thread
	int m_xmlRpcTimeoutSec;
	/*QHash<int, QFile*> m_fileVideoData;*/

	bool m_firstIFrame;
	int m_chunkCount;
	QString m_sharedAbsoluteDir;
	QList<IndexEntry_t> m_listIndicesOfIFrames;          //!< List of indices pointing to I-frames
	qint32 m_availabilityOfCams;

	Job* m_pJob;

	QDateTime m_lastEventDT;

    QNetworkReply *m_reply;

    QMap<int, UploadReply_t> m_UploadClipMap;

    SiteProperty_t m_SiteProperty;

    QList<QString> m_ClipList;

public:
	signals:
	void on_TriggerJob(Job* pJob);
    //void on_startUploadRequest(QString fileName);
	//void on_stop();

public slots:
	void stopJob();
	void suspendJob();

public:
    void startJob(Job* pJob);//, WebApiProcessor* pWebApiProcessor);

	//Job* getJobTemplate() { return m_pJob; };

	bool openClipFile()
	{
		QDateTime eventStart, eventEnd;
		eventStart = m_pJob->startDateTime;
		eventEnd = m_pJob->endDateTime;
		if ((eventStart.isValid()) && (eventEnd.isValid()) && (eventStart.secsTo(eventEnd) > 0))
		{
			foreach(int i, getChannelsFromBitmask())
			{
				QString fileName = QString("%1%2_%3_%4.H264").arg(m_pJob->fileName, QString::number(eventStart.toSecsSinceEpoch()), m_pJob->vehicleName, QString::number(i));

				m_firstIFrame = false;
				if (this->openArchiveForWriting(i, QString("%1/%2").arg(m_sharedAbsoluteDir, fileName.trimmed()), "") == true)
				{
					m_pJob->jobType = JobTypeDownloadVideo;
                    qDebug() << "Archive file " << fileName.trimmed() << " opened";
				}
				else
				{
					qDebug() << QString("Cannot create output file: Job rejected \"%1\" for %2 (%3)").arg(m_pJob->jobName, m_pJob->recorderIpAddress, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
					return false;
				}
			}
		}
        qDebug() << "Clip file count: " << m_pJob->fileVideoData.count() << " Camera count from config: " << m_pJob->cameraCount;
		//if (m_fileVideoData.count() == m_pJob->cameraCount)
		if (m_pJob->fileVideoData.count() == m_pJob->cameraCount)
			return true;
		else
			return false;
	}

	/*! This function closes an open archive.*/
    void closeArchive()
    {
        foreach(int i, getChannelsFromBitmask())
        {
            /*QFile *file = m_fileVideoData.value(i);*/
            QFile *file = m_pJob->fileVideoData.value(i);

            if (!file->isOpen())
            {
                return;
            }

            //ShellExecute(NULL, L"open", L"C:\\WINDOWS\\system32\\cmd.exe", L"/C C:\\Images\\sparseRecon64.bat", L"C:\\Images\\", SW_SHOWNORMAL);
            //QProcess *process = new QProcess(this);
            //QString file = QDir::homepath + "/file.exe";
            //process->start(file);

            m_mutexFFMPEG.lock();
            QString command = "ffmpeg -i " + file->fileName() + " -vcodec copy " + file->fileName().replace(".H264", ".mp4", Qt::CaseInsensitive);
            QProcess::execute(command);
            m_mutexFFMPEG.unlock();

            //connect(&QProcess::execute, SIGNAL(finished(int)), SLOT(uploadClip(int)));
            //this->doUploadClipRequest(file->fileName().replace(".H264", ".mp4", Qt::CaseInsensitive));

            //this->doUploadClipRequest(file->fileName().replace(".H264", ".mp4", Qt::CaseInsensitive));
            QString clipName = file->fileName().replace(".H264", ".mp4", Qt::CaseInsensitive);
            //WebApiProcessor wap;
            //this->thread()->msleep(300);
            m_ClipList.append(clipName);
            //QFuture<void> uploadClipJob = QtConcurrent::run(&m_WebApiProcessor, &WebApiProcessor::doUploadClipRequest, clipName);
            //uploadClipJob.waitForFinished();
            //WebApiProcessor* wap = new WebApiProcessor(m_pJob);//, QThread::currentThread());
            //wap->doUploadStatusRequest(102);
            //wap->doUploadClipRequest(clipName);
            //emit on_startUploadRequest(clipName);

            qDebug() << "H264 file closed: " << file->fileName() << " Converted to: " << file->fileName().replace(".H264", ".mp4", Qt::CaseSensitive);
            // close data file
            file->close();
            file->remove();
/*
            if(m_ClipList.size() == m_pJob->cameraCount) {
                for(int i = 0; i < m_ClipList.size(); i++) {
                    QFuture<void> uploadClipJob = QtConcurrent::run(&m_WebApiProcessor, &WebApiProcessor::doUploadClipRequest, m_ClipList[i]);
                    QThread::msleep(1000);
                }
                m_ClipList.clear();
            }
  */
        }
        /*m_fileVideoData.clear();*/
        m_pJob->fileVideoData.clear();
    }

	/*! This function removes an open or closed archive.*/
	void removeArchive()
	{
		foreach(int i, getChannelsFromBitmask())
		{
			/*QFile *file = m_fileVideoData.value(i);*/
			QFile *file = m_pJob->fileVideoData.value(i);
			
			if (!file->isOpen())
			{
				file->close();
			}
			// remove file
			file->remove();
		}
	}

	void doGetVideoJob(Job* pJob);

    void setSiteProperty(Job* pJob) { m_SiteProperty = pJob->SiteProperty; }

private:
	//Job * getNextContactEventJob();

    //QByteArray XFetcher::buildContent(QString fileName);

	unsigned long processVideoChunk(QByteArray dataVideo, bool isOnlyOneIFrameWanted = false);

	//////////////////////////////////////////////////

	QList<int> getChannelsFromBitmask()
	{
		QList<int> list;
		for (int i = 0; i < 8; i++)
		{
			unsigned int cameraChannel = (1 << i);
			if (m_pJob->cameraWantedBitmask & cameraChannel)
			{
				list.append(i);
			}
		}
		return list;
	}

	/*! This function returns the file name of the archive.
		*  \return string containing file name */
	QString absoluteFilePath()
	{
		foreach(int i, getChannelsFromBitmask())
		{
			QFile *file = m_pJob->fileVideoData.value(i);//m_fileVideoData.value(i);
			return file->fileName();
		}
	}

	/*! This function returns true, if video data has been received during download process.
	*  \return true, if video data received, false, if no video data received */
	bool checkVideoData()
	{
		return (m_listIndicesOfIFrames.count() > 0);
	}

	/*! This function writes index entries to the open archive.
	*  \param timeStamp date/time of referenced key frame
	*  \param camera    camera number (starting from 0) */
	void writeIndexToArchive(qint64 timeStamp, int camera)
	{
		// store time stamp and position in list of indices
		IndexEntry_t indexEntry;
		indexEntry.timeStamp = timeStamp;

		foreach(int i, getChannelsFromBitmask())
		{
			/*QFile *file = m_fileVideoData.value(i);*/
			QFile *file = m_pJob->fileVideoData.value(i);
			
			indexEntry.filePos = file->pos();
		}
		indexEntry.cameraPattern = (1 << camera);
		m_listIndicesOfIFrames.append(indexEntry);
	}

	/*! This function stores a pattern of 32 bits in the list of indices.
	*  Each bit gives information on the availability of the
	*  corresponding camera. So far only the 8 LSBs are used.
	*  \param availCams bit pattern of camera availability
	*                   bit 0 = 1: camera 1 available
	*                   bit 1 = 1: camera 2 available
	*              ...  bit 7 = 1: camera 8 available */
	void writeCamsToArchive(quint32 availCams)
	{
		// update bit pattern
		m_availabilityOfCams |= availCams;
	}

	bool isH264StartCodeIncluded(char* pData, int length) const;

	/*! This function writes data to the open archive.
	*  \param pData pointer to data array containing the video stream
	*  \param len   number of bytes in data array */
	void writeDataToArchive(int cameraChannel, const char *pData, int len)
	{
		// write data to video data file
		/*QFile *file = m_fileVideoData.value(cameraChannel);*/
		QFile *file = m_pJob->fileVideoData.value(cameraChannel);
		
		file->write(pData, len);
	}

	bool openArchiveForWriting(int channel, const QString & name, const QString & passhash)
	{
		/*QFile *file = m_fileVideoData.value(channel);*/
		QFile *file = new QFile(name);
		// create data file
		//file->setFileName(name);
		if (!file->open(QIODevice::WriteOnly))
		{
            qDebug() << "Cannot open: " << name;
			return false;
		}
        qDebug() << "File " << name << " opened for writing";
		/*m_fileVideoData.insert(channel, file);*/
		m_pJob->fileVideoData.insert(channel, file);
		// clear list of indices
		m_listIndicesOfIFrames.clear();
		// clear availability of cameras
		m_availabilityOfCams = 0;
		return true;
	}


};
