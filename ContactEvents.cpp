#include "ContactEvents.h"

ContactEvents::ContactEvents(ServiceSettings* pServiceSettings, QObject *parent)
	: QObject(parent),
	m_stopLoop(false),
	m_xmlRpcTimeoutSec(10)
{
    //m_xFetcher.SetSiteSettings(pServiceSettings->GetSite());
    m_pJob = pServiceSettings->GetJobFromConfig();
/*
    qDebug() << this->thread();
    qDebug() << this->parent()->thread();
    qDebug() << QCoreApplication::instance()->thread();
  */
    //m_SiteProperty = pServiceSettings->GetSite();
    connect(this, &ContactEvents::on_ContactEvent, this, &ContactEvents::on_jobStarted);
}

ContactEvents::~ContactEvents()
{
	emit on_stopJob();
}

void ContactEvents::handleContactEvents()
{
	QList<ContactEvent_t> listContactEvents;
	QDateTime start, end;
	int returnBackSec = 0;
	Job* pJob = new Job(*getJobTemplate()); // clone job template
	pJob->jobId = -1;
    //QDateTime dt = QDateTime::fromSecsSinceEpoch(1545268733).addSecs(-5);
    m_lastEventDT = QDateTime::currentDateTime().addSecs(-10);
    //m_lastEventDT = dt.addSecs(-10);
    qDebug() << m_lastEventDT.toString("yyyy/MM/dd_hh:mm:ss");
	
	while (!m_stopLoop)
	{

        start = QDateTime::currentDateTime().addSecs(-2).addMSecs(-1);
        //start = dt.addSecs(-2).addMSecs(-1);
        if (m_lastEventDT.addSecs(2) <= start) //contact debounce delay 2 sec
		{
            end = QDateTime::currentDateTime().addSecs(1).addMSecs(-1);
            //end = dt.addSecs(1).addMSecs(-1);

			int retval = RecorderAccess::getEventsContact(pJob->recorderIpAddress, pJob->senderAddress, pJob->xmlRpcPort, pJob->isEncrypted, start, end, listContactEvents, m_xmlRpcTimeoutSec);
			if (listContactEvents.count() > 0)
			{
				qDebug() << "Contact events found between: " << start.toString("yyyy/MM/dd_hh:mm:ss") << " and " << end.toString("yyyy/MM/dd_hh:mm::ss");
				foreach(ContactEvent_t pContactEvent, listContactEvents)
				{
					if (pContactEvent.localTime.addSecs(2) > m_lastEventDT
						&& pContactEvent.channel == pJob->doorOpenChannel
						&&  pContactEvent.state == pJob->triggerEventState) //state == true: open, state == false: closed 
					{
						qDebug() << "Door closed event found at: " << pContactEvent.localTime.toString("yyyy/MM/dd_hh:mm:ss");
						//pJob = new Job();
						pJob->startDateTime = pContactEvent.localTime.addSecs(pJob->clipDelayStart + returnBackSec);
						pJob->endDateTime = pJob->startDateTime.addSecs(pJob->clipDuration + returnBackSec);
                        pJob->currDateTime = QDateTime::currentDateTime();
                        pJob->currentDay = QDateTime::currentDateTime().date();
                        //pJob->currDateTime = QDateTime::fromSecsSinceEpoch(1545268733);
                        //pJob->currentDay = QDateTime::fromSecsSinceEpoch(1545268733).date();
						pJob->jobId = 1;
						pJob->jobStep = 0;
						m_lastEventDT = pJob->currDateTime;
						emit on_ContactEvent(pJob);
						break;
					}
				}
			}
		}
		QThread::currentThread()->msleep(100);
	}
}

void ContactEvents::on_jobStarted(Job* pJob)
{	
	connect( &m_xFetcher, &XFetcher::on_TriggerJob, this, &ContactEvents::processXFetcherJob); 
	//connect(this, &ContactEvents::on_ContactEvent, &m_xFetcher, &XFetcher::doGetVideoJob);
	connect(this, &ContactEvents::on_stopJob, &m_xFetcher, &XFetcher::stopJob);

    QFuture<void> contactJob = QtConcurrent::run(&m_xFetcher, &XFetcher::startJob, pJob);//, m_WebApiProcessor);
}

void ContactEvents::on_jobStopped()
{
	emit on_stopJob();
}

void ContactEvents::processXFetcherJob(Job* pJob)
{
	//m_xFetcher.doGetVideoJob(pJob);
	qDebug() << "Process #ContactEvent# Clip Download job from: " << pJob->startDateTime.toString("yyyyMMdd_hh:mm:ss") << " to " << pJob->endDateTime.toString("yyyyMMdd_hh:mm:ss");
}
