#include "ServiceSettings.h"

/*****************************************************************************
*****************************************************************************
**                            P U B L I C                                  **
*****************************************************************************
*****************************************************************************/

ServiceSettings::ServiceSettings(QObject* pParent /* = NULL */)
	: QObject(pParent)
{

	// Prepare setting sources from ini file or registry of current user
	/*QSettings settingsIniFile(QSettings::IniFormat, QSettings::SystemScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());*/
	QString iniDirPath = QCoreApplication::applicationDirPath();
	QSettings settingsIniFile(iniDirPath + "/demosettings.ini", QSettings::IniFormat);
	QSettings* pSettingsSource = &settingsIniFile;
	qDebug() << settingsIniFile.fileName();

	// Now read the settings
	pSettingsSource->beginGroup("Configuration");
	m_pJob = new Job();
	m_pJob->jobId = 1;
	m_pJob->jobName = pSettingsSource->value("jobName", "DownloadVideo").toString();
	//bool *keyDefined = false;
	//qDebug() << pSettingsSource->value("jobType", "JobTypeDownloadVideo").toString() << " ||| "
	//	<< QMetaEnum::fromType<JobType>().key(2)
	//	<< QMetaEnum::fromType<JobType>().keyToValue(pSettingsSource->value("jobType", "JobTypeDownloadVideo").toString().toStdString().c_str(), keyDefined);
	m_pJob->jobType = JobType::JobTypeDownloadVideo;//(JobType)(QMetaEnum::fromType<JobType>().keyToValue(pSettingsSource->value("jobType", "JobTypeDownloadVideo").toString().toStdString().c_str(), keyDefined));
	m_pJob->connectionLostDateTime = QDateTime::currentDateTime().addDays(-1);//pSettingsSource->value("").toDateTime();
	//m_pJob->transferMediumWantedBitmask = transferMediumWantedBitmask;
	m_pJob->jobStep = 0;
	m_pJob->vehicleName = pSettingsSource->value("vehicleName", "BerlinDemo").toString();
	m_pJob->recorderName = pSettingsSource->value("recorderName", "X-DMR").toString();
	m_pJob->recorderIpAddress = pSettingsSource->value("recorderIpAddress", "172.22.55.77").toString();
	m_pJob->senderAddress = pSettingsSource->value("senderAddress", "172.22.55.77").toString();
	m_pJob->xmlRpcPort = pSettingsSource->value("xmlRpcPort", "60998").toInt();
	m_pJob->isEncrypted = pSettingsSource->value("isEncrypted", "1").toBool();
	m_pJob->comment = pSettingsSource->value("comment", "Berlin Demo for capacity").toString();
	m_pJob->fileName = pSettingsSource->value("fileName", "clip_").toString();
	m_pJob->videoFolder = QDir::fromNativeSeparators(pSettingsSource->value("videoFolder", "C:/X-VideoClips").toString());
	m_pJob->startDateTime = QDateTime::currentDateTime();//pSettingsSource->value("").toDateTime();
	m_pJob->endDateTime = QDateTime::currentDateTime().addDays(1);//pSettingsSource->value("").toDateTime();
	m_pJob->currDateTime = QDateTime::currentDateTime();//pSettingsSource->value("").toDateTime();
	m_pJob->currentDay = QDateTime::currentDateTime().date(); //pSettingsSource->value("").toDate();
	m_pJob->cameraCount = pSettingsSource->value("cameraCount", "8").toInt();
	m_pJob->iFrameReceivedCameraBitmask = pSettingsSource->value("iFrameReceivedCameraBitmask", "3").toULongLong();
	m_pJob->cameraWantedBitmask = pSettingsSource->value("cameraWantedBitmask", "3").toULongLong();
	m_pJob->doorOpenChannel = pSettingsSource->value("doorOpenChannel", "0").toInt();
	m_pJob->clipDelayStart = pSettingsSource->value("clipDelayStart", "10").toInt();
	m_pJob->clipDuration = pSettingsSource->value("clipDuration", "20").toInt();
	m_pJob->triggerEventState = pSettingsSource->value("triggerEventState", "1").toBool();
	pSettingsSource->endGroup();

}

ServiceSettings::~ServiceSettings()
{
}



void ServiceSettings::write(Job* pJob) const
{
	QSettings settingsIniFile(QSettings::IniFormat, QSettings::SystemScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
	settingsIniFile.beginGroup("Configuration");
	settingsIniFile.setValue("jobName", pJob->jobName);
    //settingsIniFile.setValue("jobType", QString::fromUtf8(QMetaEnum::fromType<JobType>().valueToKey(pJob->jobType)));
	settingsIniFile.setValue("vehicleName", pJob->vehicleName);
	settingsIniFile.setValue("recorderName", pJob->recorderName);
	settingsIniFile.setValue("recorderIpAddress", pJob->recorderIpAddress);
	settingsIniFile.setValue("senderAddress", pJob->senderAddress);
	settingsIniFile.setValue("xmlRpcPort", pJob->xmlRpcPort);
	settingsIniFile.setValue("isEncrypted", pJob->isEncrypted ? 1 : 0);
	settingsIniFile.setValue("comment", pJob->comment);
	settingsIniFile.setValue("videoFolder", pJob->videoFolder);
	settingsIniFile.setValue("fileName", pJob->fileName);
	settingsIniFile.setValue("cameraCount", pJob->cameraCount);
    settingsIniFile.setValue("iFrameReceivedCameraBitmask", QVariant::fromValue(pJob->iFrameReceivedCameraBitmask));
	settingsIniFile.setValue("doorOpenChannel", pJob->doorOpenChannel);
	settingsIniFile.setValue("clipDelayStart", pJob->clipDelayStart);
	settingsIniFile.setValue("clipDuration", pJob->clipDuration);
	settingsIniFile.setValue("triggerEventState", pJob->triggerEventState ? 1 : 0);
	settingsIniFile.endGroup();
}
