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
    Job* GetJobFromConfig() { return m_pJob; }
    SiteProperty_t GetSite() { return m_SiteProperty; }
    //static SiteProperty_t SiteProperty;
private:
	Job* m_pJob;
    SiteProperty_t m_SiteProperty;
    SiteProperty_t GetSiteProperty(qint64 siteID, QString customerApiKey, qulonglong cameraBitMask, QList<float> cameraWeights, QString urlApi);
};
