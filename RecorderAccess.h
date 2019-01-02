#pragma once

#include <map>
#include <vector>
#include <sstream>

#include <QObject>
#include <QDateTime>
#include <QStringList>
#include <QtNetwork/QTcpSocket>

#include "Definitions.h"
#include "../XmlRpcLinux/XmlRpc.h"
#include "../XmlRpcLinux/xtea.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#define QSTRING_FROM_XMLRPC(xmlrpc) QString::fromLatin1(std::string(xmlrpc).c_str())
#define XMLRPC_FROM_QSTRING(string) QString(string).toLatin1().toStdString()
#else
#define QSTRING_FROM_XMLRPC(xmlrpc) QString::fromStdString(xmlrpc)
#define XMLRPC_FROM_QSTRING(string) QString(string).toStdString()
#endif

//! RecorderAccess class.
/*! This class offers functions to communicate with a recorder. */
class RecorderAccess : public QObject
{
	Q_OBJECT

public:
    static int getDvrInfo(QString ipAddress, QString sndAddress, int port, bool encrypt, int timeoutSec, DvrInfo& dvrInfo);

	static int getEventsContact(QString ipAddress, QString sndAddress, int port, bool encrypt, QDateTime start, QDateTime end, QList<ContactEvent_t> &listEvents, int timeoutSec);     //!< Retrieves all contact events in the given time range

	static int getVideoChunk(QString ipAddress, QString sndAddress, int port, bool encrypt, QDateTime timeSec, bool lastChunk, QByteArray &dataVideo, int timeoutSec, long channelMask);                    //!< Retrieves video chunk of the given time range

	static int executeRpc(QString passphrase, QString ipAddress, int port, const char* command, XmlRpc::XmlRpcValue & args, XmlRpc::XmlRpcValue & result, bool encrypt, int timeoutSec, QString* pError = NULL);

	//void systemReboot();                                                 //!< Reboots the recorder
	static bool recorderIsReachable(const QString& sndAddress, int port);

private:
	static int callRpcClient(XmlRpc::XmlRpcClient & client, const char* command, XmlRpc::XmlRpcValue & args, XmlRpc::XmlRpcValue & result, int timeoutSec, QString* pError = NULL);

	static void runLengthDecode(char *inData, int inSize, char *outData, int &outSize);  //!< Decodes run-length encoded recording information
	static bool decodeRunLengthData(const XmlRpc::XmlRpcValue::BinaryData &data, char *utcMinutesOfDay);
	static bool resultIsArray(const XmlRpc::XmlRpcClient &client, const XmlRpc::XmlRpcValue &result);
	static bool resultIsBinary(const XmlRpc::XmlRpcClient &client, const XmlRpc::XmlRpcValue &result);
	static bool resultIsStruct(const XmlRpc::XmlRpcClient &client, const XmlRpc::XmlRpcValue &result);
	static int minutesOfDay(QDate date);
};
