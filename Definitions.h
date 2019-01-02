#pragma once

#include <QObject>
#include <QDateTime>
#include <QtCore>
#include <QMetaType>
#include <QNetworkAccessManager>

#define DEFAULTPORT_XMLRPC 60999

#define SYS_MESSAGE_BOX_SYSERR 1
#define SYS_MESSAGE_BOX_TOOCOLD 2
#define SYS_MESSAGE_BOX_NOMEDIA 3
#define SYS_MESSAGE_BOX_MEDIAERR 5
#define SYS_MESSAGE_BOX_HDDERR 6
#define SYS_MESSAGE_BOX_SDCARDERR 7
#define SYS_MESSAGE_BOX_NOVIDEO 10
#define SYS_MESSAGE_BOX_MANIPULATED 15
#define SYS_MESSAGE_BOX_GPSMISSING 16
#define SYS_MESSAGE_BOX_ALARM    17
#define SYS_MESSAGE_BOX_STREAM   18
#define SYS_MESSAGE_BOX_USERDEF 19
#define SYS_MESSAGE_BOX_USERDEF2 20

// Be aware of the fact that we can only have 64 hardware OSD channels - if we're
// ever gonna use them...
#if defined DVRS_BASKET_FS || defined _WIN32 // if it's a DVRS or we're building a front end
#define SYS_MESSAGE_BOX_CNT 48
#else // it is an X-DMR
#define SYS_MESSAGE_BOX_CNT 20 // do NOT change this value without writing an upgrade procedure for the config profile of older X-DMR firmware !!
#endif

// define future run length encodings here
#define AD_RUN_LENGTH_ENCODING 1

#define DEFAULT_TCP_TIMEOUT 10 // normal timeout for xmlrpc commands
#define DEFAULT_FWUPDATE_TIMEOUT (4*60) // xmlrpc command system.update.firmware (JOB_FIRMWAREUPDATE) has 4 minutes timeout (normally needs 1:35 on a fast network)

#define TIMEOUT_FIRMWARE_UPDATE_SEC (4 * 60) // xmlrpc command system.update.firmware has 4 minutes timeout (normally needs 1:35 on a fast network)
#define TIMEOUT_CLEAR_VIDEO_WRITE_PROTECTION_SEC 30
#define TIMEOUT_SET_VIDEO_WRITE_PROTECTION_SEC 30 // TBD DirkS
#define TIMEOUT_SYNC_RECORDER_CLOCK_SEC 40
#define TIMEOUT_DOWNLOAD_RECORDER_LOGGING_SEC 45
#define MAX_IFRAME_SEARCH_OFFSET_SEC 6
#define TIMEOUT_RECORDER_RECONNECTION_CHECK_SEC 30
#define H264_START_CODE 0x000001

#define NALU_DELIMITER1 0x00000001
#define NALU_DELIMITER2 0x0910
#define NALU_DELIMITER3 0x0930
#define NALU_DELIMITER4 0x06010900
#define NALU_DELIMITER5 0x02
#define NALU_DELIMITER6 0x08246800
#define NALU_DELIMITER7 0x00030001
#define NALU_DELIMITER8 0x80

#define COMPAT_NO_OF_CAMS 8
#define NUMBER_OF_SECONDS (60 * 60 * COMPAT_NO_OF_CAMS)
#define NUMBER_OF_MINUTES (60 * 25 * COMPAT_NO_OF_CAMS)

#define CONNTIMEOUT       2000   //!< Time-out in msec for connection to recorder

#define XMLRPC_PORT       60999  //!< Default port number for XML RPC

#define RECORDERACCESS_RETVAL_OK        0x00000000  //!< No error
#define RECORDERACCESS_RETVAL_NOCONN    0x00001000  //!< No connection / connection broken
#define RECORDERACCESS_RETVAL_NOSUPP    0x00001001  //!< Function not supported
#define RECORDERACCESS_RETVAL_PROTOCOL  0x00001002  //!< protocol error
/////////////////////////////////
#define CONNTIMEOUT       2000   //!< Time-out in msec for connection to recorder

#define XMLRPC_PORT       60999  //!< Default port number for XML RPC

#define RECORDERACCESS_RETVAL_OK        0x00000000  //!< No error
#define RECORDERACCESS_RETVAL_NOCONN    0x00001000  //!< No connection / connection broken
#define RECORDERACCESS_RETVAL_NOSUPP    0x00001001  //!< Function not supported
#define RECORDERACCESS_RETVAL_PROTOCOL  0x00001002  //!< protocol error

typedef std::map<int, std::vector<char> > RdbMap;

//! Data type for entries of HDD history
typedef struct
{
	QDateTime   time;
	QString     macAddress;
	QString     deviceName;
	QString     vehicleName;
	QStringList cameraNames;
} MediumHistoryEntry_t;

//! Data type for contact events
typedef struct
{
	QDateTime               localTime;                         //!< Local time of occurrence
	bool                    state;                             //!< Contact state
	int                     channel;                           //!< Contact channel
	QString                 name;                              //!< Contact name
} ContactEvent_t;

enum JobType
{
	JobTypeAny,
	JobTypeDownloadFlipbook,
	JobTypeDownloadVideo,
	JobTypeQueryVideoWriteProtection,
	JobTypeAutomaticDownloadVideo,
	JobTypeSetVideoWriteProtection,
	JobTypeClearVideoWriteProtection,
	JobTypeUpdateRecorder,
	JobTypeSyncRecorderClock,
	JobTypeQueryCameraNames,
	JobTypeDownloadRecorderLogging,
	JobTypeRequestBeacon
};

//Q_DECLARE_METATYPE(JobType)

typedef struct DvrInfo
{
	QString deviceType;			//<std::string> Type of the device, e.g.X - DMR 8.
	QString	deviceName;			//<std::string> User selected name of the device
	int	hwRevisionInt;			//<int> Revision no of the hardware
	int	fwVersionInt;			//<int> Firmware version 32 bit: 31..24 = Major, 23..16 : Minor, 15..0 : Release
	int	stmFwVerInt;			//<int> STM32 Firmware Version(MSW:Minor, LSW : Major), swapped, just for backwards compatibility
	int	stmProtVerInt;			//<int> STM32 Protocol Version(MSW:Minor, LSW : Major), swapped, just for backwards compatibility

	int	stmFwVerMaj;			//<int> STM32 Firmware Version Major
	int	stmFwVerMin;			//<int> STM32 Firmware Version Minor
	int stmProtVerMaj;			//<int> STM32 Protocol Version Major
	int	stmProtVerMin;			//<int> STM32 Protocol Version Minor
	QString	buildDate;			//<std::string> Build date and time as string
	QString	revisionDate;		//<std::string> date and time as string of the last source checkin
	QString	revisionInfo;		//<std::string> Subversion revision info as string
	int	channelCount;			//<int> number of available hardware camera channels, 0..8
	int	chCountLive;			//<int> number of actually available camera channels for live view, 0..8
	int	chCountPlay;			//<int> number available camera channels for playback of recordings, 2..8
	int	resolutionIdx;			//<int> values from RESOLUTION_2CH_D1..RESOLUTION_4D1_PLUS_4CIF_B, see define.h Fundamental values are : RESOLUTION_4CH_CIF = 1; RESOLUTION_8CH_CIF = 2; RESOLUTION_4CH_D1 = 3; RESOLUTION_8CH_HALF_D1 = 8.
	int	videoSystem;			//<int> INPUT_PAL = 1 or INPUT_NTSC = 0, see define.h
	int	noSignalMask;			//<int> bitmask for all channels on which channel is the video signal missing : LSB = channel 0, 1 means missing, 0 means signal is OK or cha
	int	systemTime;				//<int> The actual time of the device in seconds since 1970 - 01 - 01 (UTC)
	int	utcTimeYear;			//<int> The actual year(1900 - 2037)
	int	utcTimeMonth;			//<int> The actual month(1..12)
	int	utcTimeDay;				//<int> actual day of month 1..31
	int	utcTimeHour;			//<int> actual hour 0..23
	int	utcTimeMinute;			//<int> actual minute 0..59
	int	utcTimeSecond;			//<int> actual second 0..59
	bool		dhcp;			//<bool> 0 / 1
	QString	ipAddressStr;		//<std::string> e.g. 192.168.1.250
	QString	netMaskStr;			//<std::string> e.g. 255.255.255.0
	QString	gatewayStr;			//<std::string> e.g.192.168.1.1
	int	basePortNo;				//<int> 0..65535, first IP - port no, the next three are used also
	QString	macAddrStr;			//<std::string>
	int	encoderType;		//<std::string> 0 = H.264; 1 = MPEG4, 2 = MJPEG(see ALG_VID_CODEC_XXX in alg.h)
} DvrInfo;

typedef struct
{
    //int Sequence_Index;
    qint64  AssetID;
    qint64  Status;
    QString ErrMessage;
} UploadReply_t;

typedef struct
{
   qint64  videoID; //<ID (int)> // This is the asset ID
   qint64  assetStatus; //<status code int>
   QString ErrMessage;
} UploadStatus_t;

//! Data type of index entry for I-frame index list
typedef struct
{
    qint64 SiteID;          //<site(bus) id>
    qint64 AssetIDs[10];    //[<id1>, <id2>, ..., <idn>]
    qint64 CameraIDs[10];   //[<id1>, <id2>, ..., <idn>]
    qint64 Weights[10];     //[<w1>, <w2>, ..., <wn>]
} CapacityRequest_t;

typedef struct
{
    //QString RequestStatus;
    qint64  CapacityID;
    qint64  Status;
    QString  ErrMessage;
} CapacityReply_t;

typedef struct
{
    qint64 CapacityID;      //<capacity id from IAPI>
    qint64 CameraIDs[10];   //[<id1>, <id2>, ..., <idn>]
    qint64 Counts[10];      //[<count1>, <count2>, ..., <countn>]
    qint64 Total;           //<total>
} CapacityStatusReply_t;

typedef struct
{
    qint64 channelID;
    QString label;
    float weight;
    bool enabled;
} CameraProperty_t;

typedef struct
{
  qint64 siteID;
  QString customerApiKey;
  QString urlApi;
  QList<CameraProperty_t> cameras;
} SiteProperty_t;

//static SiteProperty_t SiteProperty;

typedef struct Job
{
    int jobId;
    QString jobName;
    JobType jobType;
    QDateTime connectionLostDateTime;
    int jobStep;
    QString vehicleName;
    QString recorderName;
    qint64 siteID;
    QString customerApiKey;
    QString urlApi;
    QList<float> cameraWeights;
    QString recorderIpAddress;
    QString senderAddress;
    int xmlRpcPort;
    bool isEncrypted;
    QString comment;
    QString fileName;
    QString videoFolder;
    QHash<int, QFile*> fileVideoData;
    QDateTime startDateTime;
    QDateTime endDateTime;
    QDateTime currDateTime;
    QDate currentDay;
    QList<ContactEvent_t> listContactEvents;
    QList<QList<char>> listTimes;
    int cameraCount;
    unsigned long iFrameReceivedCameraBitmask;
    unsigned long cameraWantedBitmask;
    qint8 doorOpenChannel;
    qint16 clipDelayStart;
    qint16 clipDuration;
    bool triggerEventState;
    SiteProperty_t SiteProperty;
    //// Copy constructor
    //Job(const Job & j) :
    //	jobId(j.jobId),
    //	jobName(j.jobName),
    //	jobType(j.jobType),
    //	connectionLostDateTime(j.connectionLostDateTime),
    //	jobStep(j.jobStep),
    //	vehicleName(j.vehicleName),
    //	recorderName(j.recorderName),
    //	recorderIpAddress(j.recorderIpAddress),
    //	senderAddress(j.senderAddress),
    //	xmlRpcPort(j.xmlRpcPort),
    //	isEncrypted(j.isEncrypted),
    //	comment(j.comment),
    //	fileName(j.fileName),
    //	videoFolder(j.videoFolder),
    //	startDateTime(j.startDateTime),
    //	endDateTime(j.endDateTime),
    //	currDateTime(j.currDateTime),
    //	currentDay(j.currentDay),
    //	cameraCount(j.cameraCount),
    //	iFrameReceivedCameraBitmask(j.iFrameReceivedCameraBitmask),
    //	doorOpenChannel(j.doorOpenChannel),
    //	clipDelayStart(j.clipDelayStart),
    //	clipDuration(j.clipDuration),
    //	triggerEventState(j.triggerEventState)
    //{};
    //Job& operator=(const Job& j)
    //{
    //	jobId = j.jobId;
    //	jobName = j.jobName;
    //	jobType = j.jobType;
    //	connectionLostDateTime = j.connectionLostDateTime;
    //	jobStep = j.jobStep;
    //	vehicleName = j.vehicleName;
    //	recorderName = j.recorderName;
    //	recorderIpAddress = j.recorderIpAddress;
    //	senderAddress = j.senderAddress;
    //	xmlRpcPort = j.xmlRpcPort;
    //	isEncrypted = j.isEncrypted;
    //	comment = j.comment;
    //	fileName = j.fileName;
    //	videoFolder = j.videoFolder;
    //	startDateTime = j.startDateTime;
    //	endDateTime = j.endDateTime;
    //	currDateTime = j.currDateTime;
    //	currentDay = j.currentDay;
    //	cameraCount = j.cameraCount;
    //	iFrameReceivedCameraBitmask = j.iFrameReceivedCameraBitmask;
    //	doorOpenChannel = j.doorOpenChannel;
    //	clipDelayStart = j.clipDelayStart;
    //	clipDuration = j.clipDuration;
    //	triggerEventState = j.triggerEventState;
    //	return *this;
    //};
} Job;


static QQueue<QMap<int, UploadReply_t>> AssetsQueue;

static QQueue<QMap<int, CapacityStatusReply_t>> CapacityStatusQueue;

static QNetworkAccessManager AccessManager;
