#pragma once

#include <QObject>
#include <QDebug>
#include <QSettings>
#include <QMetaObject>
#include <QCoreApplication>
#include <QDir>
#include <QMetaType>
#include "Definitions.h"

class ServiceSettings : public QObject
{
	Q_OBJECT
public:
	//#define JobType JobType
	//enum JobType
	//{
	//	JobTypeAny,
	//	JobTypeDownloadFlipbook,
	//	JobTypeDownloadVideo,
	//	JobTypeQueryVideoWriteProtection,
	//	JobTypeAutomaticDownloadVideo,
	//	JobTypeSetVideoWriteProtection,
	//	JobTypeClearVideoWriteProtection,
	//	JobTypeUpdateRecorder,
	//	JobTypeSyncRecorderClock,
	//	JobTypeQueryCameraNames,
	//	JobTypeDownloadRecorderLogging,
	//	JobTypeRequestBeacon
	//};
	Q_ENUM(JobType)
	ServiceSettings(QObject *parent);
	~ServiceSettings();
	void write(Job* pJob) const;
	Job* GetJobFromConfig() { return m_pJob; };
private:
	Job* m_pJob;
};
