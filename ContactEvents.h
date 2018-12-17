#pragma once

#include <QObject>
#include <QDebug>
#include <QCoreApplication>
#include <QThread>
#include <QDateTime>
#include <QFile>
#include <QMutex>
#include <QDataStream>
#include "qobjectdefs.h"
#include <QFuture>
#include <QtConcurrent/QtConcurrentRun>

#include "fcommand.h"
#include "Definitions.h"
#include "RecorderAccess.h"
#include "../XmlRpcLinux/XmlRpcValue.h"
#include "ServiceSettings.h"
#include "XFetcher.h"

class ContactEvents : public QObject
{
	Q_OBJECT

public:
	explicit ContactEvents(ServiceSettings* pServiceSettings, QObject *parent);
	~ContactEvents();

public:
	void handleContactEvents();

signals:
	void on_stopJob();
	void on_ContactEvent(Job* pJob);

private slots:
	void on_jobStopped();
	void on_jobStarted(Job* pJob);

public slots:
	void processXFetcherJob(Job* pJob);

private:
	int m_xmlRpcTimeoutSec;
	XFetcher m_xFetcher;
	Job* m_pJob;
	bool m_stopLoop;
	QDateTime m_lastEventDT;

private:
	Job* getJobTemplate() { return m_pJob; };
};
