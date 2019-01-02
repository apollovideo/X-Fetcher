#include "XFetcher.h"

XFetcher::XFetcher(QObject* pParent/* = NULL*/)
	: QObject(pParent),
	m_isStopThreadWanted(false),
	m_isSuspendThreadWanted(false),
    m_xmlRpcTimeoutSec(10),
    m_WebApiProcessor(pParent)
{
	//m_pJob = pServiceSettings->GetJobFromConfig();
	//connect(this, SIGNAL(triggerJob(Job*)), this, SLOT(onTriggerJob(Job*)));
	//connect(this, &XFetcher::on_stop, this, &XFetcher::stopJob);
    //m_ServiceSettings = new ServiceSettings(pParent);//(QCoreApplication::instance()->thread());//
    //m_WebApiProcessor = new WebApiProcessor(pParent);
    //connect(this, &XFetcher::on_startUploadRequest, &m_WebApiProcessor, &WebApiProcessor::doTest, Qt::QueuedConnection);
    //connect(this, SIGNAL(on_startUploadRequest(QString)), this, SLOT(doTest(QString)));//, Qt::QueuedConnection);
    m_ClipList.clear();
    qDebug() << "WebApiProcessor SiteProperty cameras: " << m_SiteProperty.cameras.size();
}

XFetcher::XFetcher(Job* pJob, QObject* pParent/* = NULL*/ )
    : QObject(pParent),
    m_isStopThreadWanted(false),
    m_isSuspendThreadWanted(false),
    m_xmlRpcTimeoutSec(10),
    m_SiteProperty(pJob->SiteProperty),
    m_WebApiProcessor(pJob, pParent)
{
    //m_WebApiProcessor = new WebApiProcessor(pJob, pParent);
    m_ClipList.clear();
    qDebug() << "WebApiProcessor with Job SiteProperty cameras: " << m_SiteProperty.cameras.size();
}

XFetcher::~XFetcher()
{
	//delete m_pJob;
}

bool XFetcher::isH264StartCodeIncluded(char* pData, int length) const
{
	int offset = 0;
	if ((pData[0] == 0) && (pData[1] == 0) && (pData[2] == 0) && (pData[3] == 1))
	{
		offset = 4;
	}
	else if ((pData[0] == 0) && (pData[1] == 0) && (pData[2] == 1))
	{
		offset = 3;
	}
	unsigned long searchValue = 0xffffffff;
	while (offset < length - 3)
	{
		searchValue <<= 8;
		searchValue |= *(pData + offset);
		if ((searchValue == H264_START_CODE) || ((searchValue & 0x00ffffff) == H264_START_CODE))
		{
			return true;
		}
		offset++;
	}
	return false;
}

/*! This is the thread running the job scheduler.
*  It carries out batch jobs, which communicate with recorders
*  over an unstable network link. */
void XFetcher::startJob(Job* pJob)//, WebApiProcessor* pWebApiProcessor)
{
	m_isStopThreadWanted = false;
	m_isSuspendThreadWanted = false;
	m_pJob = pJob;

    this->setSiteProperty(pJob);
    this->m_WebApiProcessor.setSiteProperty(pJob);

    //m_WebApiProcessor = new WebApiProcessor(pJob);

    //m_WebApiProcessor->setSiteProperty(pJob);
    //qDebug() << m_WebApiProcessor->thread() << " | " << this->thread();
    //m_WebApiProcessor->moveToThread(QApplication::instance()->thread());  // Move WebApiProcessor back to the original thread
    //m_WebApiProcessor->setParent(QApplication::instance()->thread());
    //QScopedPointer<WebApiProcessor> wap(new WebApiProcessor(pJob));

    int recordAvailableDelaySec = 5;
	//m_fileVideoData.reserve(m_pJob->cameraCount);
	m_pJob->fileVideoData.reserve(m_pJob->cameraCount);
	m_sharedAbsoluteDir = m_pJob->videoFolder;
	if (pJob->jobId != -1)
	{
		try
		{
			m_lastEventDT = QDateTime::currentDateTime().addSecs(-10);
            pJob->currDateTime = QDateTime::currentDateTime();
			pJob->currentDay = QDateTime::currentDateTime().date();

			DvrInfo dvrInfo;
            int result = RecorderAccess::getDvrInfo(m_pJob->recorderIpAddress, m_pJob->senderAddress, m_pJob->xmlRpcPort, m_pJob->isEncrypted, m_xmlRpcTimeoutSec, dvrInfo);
			// while scheduler not stopped
			while (!m_isStopThreadWanted)
			{
				QMutexLocker locker(&mutexJobScheduler);
				// if scheduler not suspended
				if (!m_isSuspendThreadWanted 
					&& pJob->startDateTime.addSecs(recordAvailableDelaySec) < QDateTime::currentDateTime()
					//&& pJob->endDateTime.addSecs(recordAvailableDelaySec) > QDateTime::currentDateTime()
					)
				{
					switch (pJob->jobType)
					{
					case JobTypeDownloadVideo:
					case JobTypeAutomaticDownloadVideo:
                        doGetVideoJob(pJob);
                        emit on_TriggerJob(pJob);
						break;
					default:
						break;
					}
				}
                // make the thread sleep for 10ms
                QThread::msleep(10);
			} // while (!m_isStopThreadWanted)
		}
		catch (...)
		{

		}
	}
}

void XFetcher::stopJob()
{
	// Set stop flag and wait for thread termination
	m_isStopThreadWanted = true;
	closeArchive();
}

/*! This function suspends the job scheduler. */
void XFetcher::suspendJob()
{
	m_isSuspendThreadWanted = true;  // set suspend flag
}

unsigned long XFetcher::processVideoChunk(QByteArray dataVideo, bool isOnlyOneIFrameWanted /* = false */)
{
	typedef struct
	{
		quint16 identifier;   ///< PACKET_IDENTIFIER, see fcommand.h
		quint16 cmd;          ///< CMD_XXX, e.g. CMD_SINGLE_PB_VIDEO_DATA
		quint16 length;       ///< number of bytes including this header
		quint8 fragmentCnt;   ///< total number of fragments that belong to this frame (1...?)
		quint8 fragmentIdx;   ///< 0..fragmentCnt-1
		quint32 timestamp;    ///< relative timestamp in millisec, reference does not matter. obsolete for playback, used for live streaming (see VSTREAMDATA)
		quint16 channel;      ///< 0..MAX_CHANNEL-1
		quint16 resolution;   ///< for video: resulution index, PAL720576 etc., see fdefine.h. for audio: samplingrate. actually unused, but written to exported files. Unused even there yet.
		quint32 unixSec;      ///< absolute timestamp of the recording in seconds since epoch
		quint32 unixMicrosec; ///< microsec since unixSec
	} PvStreamData;

	typedef struct
	{
		quint16 ch;           ///< video channel no 0..MAX_CHANNEL-1
		quint16 type;         ///< IFRAME = 1 or PFRAME = 0, see fdefine.h
		quint32 frameSize;    ///< number of bytes in following frame data (without this header)
	} Intimeheader;

	unsigned long iFrameReceivedCameraBitmask = 0;

	QDataStream in(&dataVideo, QIODevice::ReadOnly);
	in.setVersion(QDataStream::Qt_5_10);
	while (!in.atEnd())
	{
		// Get and store PvStreamData
		PvStreamData pvStreamData;
		in >> pvStreamData.identifier;
		if (pvStreamData.identifier != PACKET_IDENTIFIER)
		{
			return iFrameReceivedCameraBitmask;
		}
		in >> pvStreamData.cmd >> pvStreamData.length >> pvStreamData.fragmentCnt >> pvStreamData.fragmentIdx >> pvStreamData.timestamp >> pvStreamData.channel >> pvStreamData.resolution >> pvStreamData.unixSec >> pvStreamData.unixMicrosec;
		QByteArray header;
		QDataStream outHeader(&header, QIODevice::WriteOnly);
		outHeader.setVersion(QDataStream::Qt_5_10);
		outHeader << pvStreamData.identifier << pvStreamData.cmd << pvStreamData.length << pvStreamData.fragmentCnt << pvStreamData.fragmentIdx << pvStreamData.timestamp << pvStreamData.channel << pvStreamData.resolution << pvStreamData.unixSec << pvStreamData.unixMicrosec;
		int headerLength = 24;
		bool isIFrame = false;
		bool isPFrame = false;

		// Check if playback data and first fragment
		if ((pvStreamData.cmd == CMD_SINGLE_PB_VIDEO_DATA) && (pvStreamData.fragmentIdx == 0))
		{
			// Get and store Intimeheader
			Intimeheader intimeheader;
			in >> intimeheader.ch >> intimeheader.type >> intimeheader.frameSize;
			outHeader << intimeheader.ch << intimeheader.type << intimeheader.frameSize;
			headerLength = 32;

			// In case of iFrame
			if (intimeheader.type == 1)
			{
				isIFrame = true;
				isPFrame = false;

				// Write index to local archive
				this->writeIndexToArchive(((qint64)pvStreamData.unixSec * 1000) + (pvStreamData.unixMicrosec / 1000), pvStreamData.channel);

				// Write availability of camera to local archive
				this->writeCamsToArchive(1 << pvStreamData.channel);

				m_firstIFrame = true;
			}
			else if (intimeheader.type == 0)
			{
				isIFrame = false;
				isPFrame = true;
			}
		}

		// Read video data
		char *pData = new char[pvStreamData.length - headerLength];
		in.readRawData(pData, pvStreamData.length - headerLength);

		//// igor hack 2
		//quint32     *pKeyXtea = NULL;    //!< Pointer to XTEA key
		//SymmKey128  keyXtea;             //!< XTEA key for encryption of control (XMLRPC) and video data
		//{
		//	pKeyXtea = keyXtea;
		//	QString ipAddress = "172.22.55.77";
		//	XmlRpc::XmlRpcClient::calculateKeyFromPassphrase(ipAddress.toLocal8Bit().constData(), keyXtea);
		//	xtea_decrypt_stream(pKeyXtea, (pvStreamData.length - headerLength), NULL, (quint32 *)pData);
		//}
		//// igor end hack
		//qDebug() << " clip channel: " << pvStreamData.channel << " camera bitmask: " << iFrameReceivedCameraBitmask;
		if ((/*m_firstIFrame && */(isIFrame || isPFrame)) || ((isIFrame) && ((iFrameReceivedCameraBitmask & (1 << pvStreamData.channel)) == 0) && isH264StartCodeIncluded(pData, pvStreamData.length - headerLength)))
		{
			if (!isOnlyOneIFrameWanted)
			{
				//// TBD DirkS: For testing purpose
				// QFile file(QString("D:/X-VideoClips/cameraId%1_%2.H264").arg(pvStreamData.channel).arg(QDateTime::fromTime_t(pvStreamData.unixSec).time().toString(Qt::SystemLocaleLongDate).replace(':', '_').replace(' ', '_')));
				//if (file.open(QIODevice::WriteOnly) == true)
				//{
				//    file.write(pData, pvStreamData.length - headerLength);
				//    file.close();
				//} 
				//QByteArray delimHeader;
				//QDataStream outHeader1(&delimHeader, QIODevice::WriteOnly);
				//outHeader1.setVersion(QDataStream::Qt_4_7);
				///*
				//#define NALU_DELIMITER1 0x00000001
				//#define NALU_DELIMITER2 0x0910
				//#define NALU_DELIMITER3 0x0930
				//#define NALU_DELIMITER4 0x06010900
				//#define NALU_DELIMITER5 0x02
				//#define NALU_DELIMITER6 0x08246800
				//#define NALU_DELIMITER7 0x00030001
				//#define NALU_DELIMITER8 0x80
				//*/
				//if (isIFrame) {
				//	outHeader1 << (qint32)NALU_DELIMITER1 << (qint16)NALU_DELIMITER2;
				//}					
				//else {
				//	outHeader1 << (qint32)NALU_DELIMITER1 << (qint16)NALU_DELIMITER3 
				//		<< (qint32)NALU_DELIMITER1 << (qint32)NALU_DELIMITER4 << (qint8)m_PFrame_Cnt
				//		<< (qint32)NALU_DELIMITER6 << (qint32)NALU_DELIMITER7 << (qint8)NALU_DELIMITER8;	
				//						
				//	m_PFrame_Cnt += 2;
				//}

				//pStreamFileExporter->writeDataToArchive(delimHeader.data(), delimHeader.size());

				this->writeDataToArchive(pvStreamData.channel, pData, pvStreamData.length - headerLength);

				iFrameReceivedCameraBitmask |= (1 << pvStreamData.channel);
			}
		}
		delete[] pData;
	}
	return iFrameReceivedCameraBitmask;
}

/*! This function carries out a job to retrieve video from a recorder.
*  \param job         job structure of type JobTypeDownloadVideo or JobTypeAutomaticDownloadVideo */
void XFetcher::doGetVideoJob(Job* pJob)
{
	QStringList             listCameraNames;
	QByteArray              dataVideo;
	QDateTime               start, end;
	bool                    lastChunk;
	int                     orientation = -1;
	char                    minutes[NUMBER_OF_MINUTES];

	//QMutexLocker locker(&mutexJobProcessing); 

	qDebug() << "Step: " << pJob->jobStep;

	// check, if job type is ok
	if ((pJob->jobType != JobTypeDownloadVideo) && (pJob->jobType != JobTypeAutomaticDownloadVideo))
	{
		qDebug() << "Wrong job type";
		return;
	}

	start = pJob->currDateTime;

	// determine time range of data to be requested
	if (pJob->jobStep == 0)  // first 2 steps and 4th to 7th step and 9th step to get events
	{
		end = start.addDays(1).addMSecs(-1);
		if (end >= pJob->endDateTime)
		{
			end = pJob->endDateTime;
		}
	}
	else if (pJob->jobStep == 1)
	{
		m_chunkCount = 0;

		// do we fetch the data of the last day we need?
		if (pJob->currentDay == pJob->endDateTime.date())
			end = pJob->endDateTime; // jupp
		else
			end = pJob->startDateTime; // no
	}
	else  // last step to get video chunks
	{
		m_chunkCount++;
		end = start.addSecs(1).addMSecs(-1);
		if (end >= pJob->endDateTime)
		{
			lastChunk = true;
		}
		else
		{
			lastChunk = false;
		}		
		
		if (m_chunkCount == 1)
			qDebug() << "Job started at: " << QDateTime::currentDateTime().toString("yyyy/MM/dd_hh:mm:ss");
		qDebug() << "Chunk count: " << m_chunkCount << ". Last chunk: " << lastChunk;
	}

	int retval;
	switch (pJob->jobStep)
	{
	case 0:
		if (openClipFile())
			retval = 0;
		else
			retval = 1;
		qDebug() << "Call openArchive";
		//qDebug() << "Current time: " << QDateTime::currentDateTime().toString("yyyyMMdd_hh:mm:ss");
		qDebug() << "Job: from " << pJob->startDateTime.toString("yyyyMMdd_hh:mm:ss") << " to " << pJob->endDateTime.toString("yyyyMMdd_hh:mm:ss");
		break;
	case 1:
	{
		// reset speed
		XmlRpc::XmlRpcValue xmlRpcArguments;
		xmlRpcArguments[0] = -30;// 0;
		XmlRpc::XmlRpcValue xmlRpcResult;
		retval = RecorderAccess::executeRpc(pJob->recorderIpAddress, pJob->senderAddress, pJob->xmlRpcPort, QString("playback.setSpeed").toStdString().c_str(), xmlRpcArguments, xmlRpcResult, pJob->isEncrypted, m_xmlRpcTimeoutSec);
	}
	break;

	default:
		// get video chunk from recorder
		// igor hack
		//pJob->isEncrypted = false;
		retval = RecorderAccess::getVideoChunk(pJob->recorderIpAddress, pJob->senderAddress, pJob->xmlRpcPort, pJob->isEncrypted, start, lastChunk, dataVideo, m_xmlRpcTimeoutSec, pJob->cameraWantedBitmask);
		qDebug() << "getVideoChunk chunkCount=" << m_chunkCount << " lastChunk=" << lastChunk << " dataVideo length=" << dataVideo.count();
	}

	// if XML RPC was successful
	if (retval == 0)
	{
		switch (pJob->jobStep)
		{
		case 0:
		case 1:
			break;
		default:
			// process and store video
			//if(pJob->startDateTime <= start && pJob->endDateTime >= start)
			processVideoChunk(dataVideo);
			qDebug() << "processVideoChunk chunkCount=" << m_chunkCount << " lastChunk=" << lastChunk;
		}
	}
	else
	{
		if (pJob->jobStep == 0)
		{
			qDebug() << "cannot create clip file";
			m_isStopThreadWanted = true;
			return;
		}
		// if connection broken
		qDebug() << "connection broken";
		if (retval == RECORDERACCESS_RETVAL_NOCONN)
		{
			pJob->connectionLostDateTime = QDateTime::currentDateTime();
			m_isStopThreadWanted = true;
			return;
		}
		else if ((pJob->jobStep > 0))
		{

			// Remove job
			//removeCurrentJob(pJob);
			m_isStopThreadWanted = true;
			return;
		}
	}

	// if finished
	if (end >= pJob->endDateTime)
	{
		switch (pJob->jobStep)
		{
		case 0:
		case 1:
			//pJob->listContactEvents.clear();

			// increment job step and reset currently processed date + time
			pJob->jobStep++;
			pJob->currDateTime = pJob->startDateTime;
			m_chunkCount = 0;
			break;
		default:
			// close file
			this->closeArchive();
			qDebug() << "call closeArchive";

			// Check if the output file contains a valid index 
			if (this->checkVideoData())
			{
                if(m_ClipList.size() == m_pJob->cameraCount) {
                    QFuture<void> uploadClipJob;
                    for(int i = 0; i < m_ClipList.size(); i++) {
                        uploadClipJob = QtConcurrent::run(&m_WebApiProcessor, &WebApiProcessor::doUploadClipRequest, m_ClipList[i]);
                        //while(!uploadClipJob.isFinished()) {
                            QThread::msleep(1000);
                        //}
                    }
                    m_ClipList.clear();
                }
				qDebug() << pJob->jobId << " Job finished at: " << QDateTime::currentDateTime().toString("yyyy/MM/dd_hh:mm:ss");;// << m_fileVideoData.value(getChannelsFromBitmask().first());
			}
			else
			{
                m_ClipList.clear();
				qDebug() << "File index error";
			}

			//delete pJob;
			
			m_isStopThreadWanted = true;
			//emit on_stop();
			// Remove job
			//removeCurrentJob(pJob);
		}
	}
	else
	{
		// Step 2 and step 1 should not come here
		if (pJob->jobStep == 0)  // getting events
		{
			// increment date + time of current job
			pJob->currDateTime = start.addDays(1);
		}
		else
		{
			if (pJob->jobStep > 1)  // getting video chunks
			{
				// increment date + time of current job
				pJob->currDateTime = start.addSecs(1);

				// Update job progress
				//qDebug() << "Processing" << pJob->jobStep + ((100 - pJob->jobStep) * pJob->startDateTime.secsTo(pJob->currDateTime) / pJob->startDateTime.secsTo(pJob->endDateTime));
			}
		}
	}
}

