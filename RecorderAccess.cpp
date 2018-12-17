#include "RecorderAccess.h"

/*! This function retrieves all contact events in the given time range.
*  \param ipAddress     string containing IP address of recorder
*  \param sndAddress    string containing IP address of sender
*  \param port          port number used to reach recorder
*  \param encrypt       true, if encryption enabled
*  \param start         start date/time of time range (if invalid, no time range is used)
*  \param end           end date/time of time range (if invalid, no time range is used)
*  \param listEvents    returned list of events
*  \return error code, 0 = ok */
int RecorderAccess::getEventsContact(QString ipAddress, QString sndAddress, int port, bool encrypt, QDateTime start, QDateTime end, QList<ContactEvent_t> &listEvents, int timeoutSec)
{
	listEvents.clear();

	// set up rpc client
	quint32     *pKeyXtea = NULL;    //!< Pointer to XTEA key
	SymmKey128  keyXtea;             //!< XTEA key for encryption of control (XMLRPC) and video data
	if (encrypt)
	{
		pKeyXtea = keyXtea;
		XmlRpc::XmlRpcClient::calculateKeyFromPassphrase(ipAddress.toLocal8Bit().constData(), keyXtea);
	}

	// check, if recorder is reachable (avoiding long waiting time)
	QTcpSocket tcpSocket;
    tcpSocket.connectToHost(sndAddress, port);
	if (!tcpSocket.waitForConnected(CONNTIMEOUT))
	{
		tcpSocket.close();
		return RECORDERACCESS_RETVAL_NOCONN;
	}
	tcpSocket.close();

	// send config command
	XmlRpc::XmlRpcClient client(sndAddress.toLocal8Bit().constData(), port, 0, pKeyXtea);
	XmlRpc::XmlRpcValue  args, result;

	int continueAt = 0;
	do
	{
		args.clear();
		if (start.isValid() && end.isValid())
		{
			args["beginpos"] = int(start.toTime_t()); // to UTC
			args["endpos"] = int(end.toTime_t());   // to UTC
		}
		args["eventRes"] = 0;
		if (continueAt > 0)
		{
			args["continueAt"] = continueAt;
		}

		int retval = callRpcClient(client, "archive.metadata.contactIn.get", args, result, timeoutSec);

		if (retval)
		{
			return retval;
		}

		QDateTime dateTime;
		for (int i = 0; i < result.size(); i++)
		{
			dateTime.setTime_t(int(result[i]["time"]));  // to local time

			ContactEvent_t evt;
			evt.localTime = dateTime;
			evt.state = (int(result[i]["value"]) == 0) ? false : true;
			evt.channel = int(result[i]["channelNo"]);
			if (result[i]["name"].valid())
			{
				evt.name = QString::fromStdString(std::string(result[i]["name"]));
			}
			else
			{
				evt.name = QString();
			}
			listEvents.append(evt);
		}

		if (result.size() > 0)
		{
			if (result[0]["continueAt"].valid())
			{
				continueAt = int(result[0]["continueAt"]);
			}
			else
			{
				continueAt = 0;
			}
		}
		else
		{
			continueAt = 0;
		}
	} while (continueAt > 0);  // continue loading, if not yet all events received

	return RECORDERACCESS_RETVAL_OK;
}

/*! This function retrieves all all camera names.
*  \param ipAddress     string containing IP address of recorder
*  \param sndAddress    string containing IP address of sender
*  \param port          port number used to reach recorder
*  \param encrypt       true, if encryption enabled
*  \param names         returned list of camera names
*  \return error code, 0 = ok
*/

int RecorderAccess::getDvrInfo(QString ipAddress,
	QString sndAddress,
	int port,
	bool encrypt,
	int timeoutSec,
	DvrInfo& dvrInfo)
{

	// set up rpc client
	quint32     *pKeyXtea = NULL;    //!< Pointer to XTEA key
	SymmKey128  keyXtea;             //!< XTEA key for encryption of control (XMLRPC) and video data
	if (encrypt)
	{
		pKeyXtea = keyXtea;
		XmlRpc::XmlRpcClient::calculateKeyFromPassphrase(ipAddress.toLocal8Bit().constData(), keyXtea);
	}

	// check, if recorder is reachable (avoiding long waiting time)
	QTcpSocket tcpSocket;
	tcpSocket.connectToHost(sndAddress, port);
	if (!tcpSocket.waitForConnected(CONNTIMEOUT))
	{
		tcpSocket.close();
		return RECORDERACCESS_RETVAL_NOCONN;
	}
	tcpSocket.close();

	XmlRpc::XmlRpcClient client(sndAddress.toLocal8Bit().constData(), port, 0, pKeyXtea);
	XmlRpc::XmlRpcValue  args, resultStruct;

	// we need to get the number of cameras from the recorder first
	args.clear();
	int retval = callRpcClient(client, "system.info", args, resultStruct, timeoutSec);
	if (retval != 0)
		return retval;
	// we have a result

	dvrInfo.deviceType = QString::fromStdString(resultStruct["deviceType"]);
	dvrInfo.deviceName = QString::fromStdString(resultStruct["deviceName"]);
	dvrInfo.hwRevisionInt = (int)resultStruct["hwRevisionInt"];
	dvrInfo.fwVersionInt = (int)resultStruct["fwVersionInt"];
	dvrInfo.stmFwVerInt = (int)resultStruct["stmFwVerInt"];
	dvrInfo.stmProtVerInt = (int)resultStruct["stmProtVerInt"];

	dvrInfo.stmFwVerMaj = (int)resultStruct["stmFwVerMaj"];
	dvrInfo.stmFwVerMin = (int)resultStruct["stmFwVerMin"];
	dvrInfo.stmProtVerMaj = (int)resultStruct["stmProtVerMaj"];
	dvrInfo.stmProtVerMin = (int)resultStruct["stmProtVerMin"];
	dvrInfo.buildDate = QString::fromStdString(resultStruct["buildDate"]);
	dvrInfo.revisionDate = QString::fromStdString(resultStruct["revisionDate"]);
	dvrInfo.revisionInfo = QString::fromStdString(resultStruct["revisionInfo"]);
	dvrInfo.channelCount = (int)resultStruct["channelCount"];
	dvrInfo.chCountLive = (int)resultStruct["chCountLive"];
	dvrInfo.chCountPlay = (int)resultStruct["chCountPlay"];
	dvrInfo.resolutionIdx = (int)resultStruct["resolutionIdx"];

	dvrInfo.videoSystem = (int)resultStruct["videoSystem"];
	dvrInfo.noSignalMask = (int)resultStruct["noSignalMask"];
	dvrInfo.systemTime = (int)resultStruct["systemTime"];
	dvrInfo.utcTimeYear = (int)resultStruct["utcTimeYear"];
	dvrInfo.utcTimeMonth = (int)resultStruct["utcTimeMonth"];
	dvrInfo.utcTimeDay = (int)resultStruct["utcTimeDay"];
	dvrInfo.utcTimeHour = (int)resultStruct["utcTimeHour"];
	dvrInfo.utcTimeMinute = (int)resultStruct["utcTimeMinute"];
	dvrInfo.utcTimeSecond = (int)resultStruct["utcTimeSecond"];
	dvrInfo.dhcp = (int)resultStruct["dhcp"];
	dvrInfo.ipAddressStr = QString::fromStdString(resultStruct["ipAddressStr"]);
	dvrInfo.netMaskStr = QString::fromStdString(resultStruct["netMaskStr"]);
	dvrInfo.gatewayStr = QString::fromStdString(resultStruct["gatewayStr"]);
	dvrInfo.basePortNo = (int)resultStruct["basePortNo"];
	dvrInfo.macAddrStr = QString::fromStdString(resultStruct["macAddrStr"]);
	dvrInfo.encoderType = (int)resultStruct["encoderType"];

	return RECORDERACCESS_RETVAL_OK;
}

/*! This function retrieves a video chunk containing video of the given time range.
*  In the special case of timeSec equalling UTC zero time, the alarm recording time interval
*  of the recorder's status beacon gets reset.
*  \param ipAddress     string containing IP address of recorder
*  \param sndAddress    string containing IP address of sender
*  \param port          port number used to reach recorder
*  \param encrypt       true, if encryption enabled
*  \param timeSec       date/time of second to be retrieved
*  \param lastChunk     true, if last chunk of video download to be retrieved
*  \param dataVideo     returned data bytes
*  \return error code, 0 = ok */
int RecorderAccess::getVideoChunk(QString ipAddress, QString sndAddress, int port, bool encrypt, QDateTime timeSec, bool lastChunk, QByteArray &dataVideo, int timeoutSec, long channelMask)
{
	dataVideo.clear();

	// set up rpc client
	quint32     *pKeyXtea = NULL;    //!< Pointer to XTEA key
	SymmKey128  keyXtea;             //!< XTEA key for encryption of control (XMLRPC) and video data
	if (encrypt)
	{
		pKeyXtea = keyXtea;
		XmlRpc::XmlRpcClient::calculateKeyFromPassphrase(ipAddress.toLocal8Bit().constData(), keyXtea);
	}

	// check, if recorder is reachable (avoiding long waiting time)
	QTcpSocket tcpSocket;
	tcpSocket.connectToHost(sndAddress, port);
	if (!tcpSocket.waitForConnected(CONNTIMEOUT))
	{
		tcpSocket.close();
		return RECORDERACCESS_RETVAL_NOCONN;
	}
	tcpSocket.close();

	// send config command
	XmlRpc::XmlRpcClient client(sndAddress.toLocal8Bit().constData(), port, 0, pKeyXtea);
	XmlRpc::XmlRpcValue  args, result;

	args.clear();
	int utcSec = int(timeSec.toTime_t()); // to UTC
	// For testing: NX_LOG(LL_INFO, QString("XML RPC command for startSecond %1 = %2").arg(utcSec).arg(timeSec.toString(Qt::SystemLocaleLongDate)).toStdString().c_str());
	args["startSecond"] = utcSec;
    args["channelMask"] = (int)channelMask;
	args["wasLastChunk"] = lastChunk ? 1 : 0;
	if (utcSec == 0)
	{
		args["resetRecTimes"] = INT_MAX;  // reset alarm recording info in beacon, in case of UTC second is zero
	}
	args["fastProtocol"] = 1;  // request fast protocol

	int retval = callRpcClient(client, "download.videoChunk", args, result, timeoutSec);

	if (retval)
	{
		return retval;
	}

	// distinguish between binary types
	if (result.getType() == XmlRpc::XmlRpcValue::TypePlainBinary)
	{
		XmlRpc::XmlRpcValue::PlainBinaryData& data = result;
		// For testing: NX_LOG(LL_INFO, QString("XML RPC result plain length %1 bytes").arg(XmlRpc::XmlRpcValue::PlainBinaryData(result).size()).toStdString().c_str());
		for (int i = 0; i < int(data.size()); i++)
		{
			dataVideo.append(data.at(i));
		}
	}
	else
	{
		XmlRpc::XmlRpcValue::BinaryData& data = result;
		// For testing: NX_LOG(LL_INFO, QString("XML RPC result length %1 bytes").arg(XmlRpc::XmlRpcValue::BinaryData(result).size()).toStdString().c_str());
		for (int i = 0; i < int(data.size()); i++)
		{
			dataVideo.append(data.at(i));
		}
	}

	return RECORDERACCESS_RETVAL_OK;
}

/// @returns 0 on success or error code on failure
int RecorderAccess::executeRpc(QString passphrase, QString ipAddress, int port, const char* command, XmlRpc::XmlRpcValue& args, XmlRpc::XmlRpcValue& result, bool encrypt, int timeoutSec, QString* pError /* = NULL */)
{
	QDateTime beginDateTime = QDateTime::currentDateTime();

	// set up rpc client
	quint32     *pKeyXtea = NULL;    //!< Pointer to XTEA key
	SymmKey128  keyXtea;             //!< XTEA key for encryption of control (XMLRPC) and video data
	if (encrypt && passphrase.size() > 0)
	{
		pKeyXtea = keyXtea;
		XmlRpc::XmlRpcClient::calculateKeyFromPassphrase(passphrase.toLocal8Bit().constData(), keyXtea);
	}

	if (!recorderIsReachable(ipAddress, port)) // check, if recorder is reachable (avoiding long waiting time)
	{
		qDebug() << QString("recorder at %s is not reachable trying command '%s'").arg(ipAddress.toLocal8Bit().constData()).arg(command);
		if (pError != NULL)
		{
			*pError = "Recorder not reachable";
		}
		return RECORDERACCESS_RETVAL_NOCONN;
	}

	// send config command
	XmlRpc::XmlRpcClient client(ipAddress.toLocal8Bit().constData(), port, 0, pKeyXtea);
	int retval = callRpcClient(client, command, args, result, timeoutSec, pError);
	qDebug() << QString("XML RPC command \"%1\" took %2 seconds").arg(command).arg((double)beginDateTime.msecsTo(QDateTime::currentDateTime()) / 1000);

	if (retval == RECORDERACCESS_RETVAL_NOSUPP)
		qDebug() << "recorder at address=" << ipAddress.toLocal8Bit().constData() << " reports fault on executing command=" << command;
	else if (retval == RECORDERACCESS_RETVAL_NOCONN)
		qDebug() << "No connection to recorder at address=" << ipAddress.toLocal8Bit().constData() << " on executing command=" << command;
	return retval;
}

bool RecorderAccess::recorderIsReachable(const QString& sndAddress, int port)
{
	int retval = true;
	QTcpSocket tcpSocket;
	tcpSocket.connectToHost(sndAddress, port);
	if (!tcpSocket.waitForConnected(CONNTIMEOUT))
		retval = false;
	tcpSocket.close();
	return retval;
}

/*! This function calls the RPC client and handles errors.
*  \param client   RPC client
*  \param command  string containing RPC command
*  \param args     arguments of RPC command
*  \param result   returned result of RPC command
*  \param specialTimeout optional individual timeout in seconds, which is different to the default timeout of Configuration::m_tcpTimeout. '0' means use Configuration::m_tcpTimeout, -1 means infinite
*  \return error code, 0 = ok */
int RecorderAccess::callRpcClient(XmlRpc::XmlRpcClient & client, const char* command, XmlRpc::XmlRpcValue & args, XmlRpc::XmlRpcValue & result, int timeoutSec, QString* pError /* = NULL */)
{
	if (!client.execute(command, args, result, double(timeoutSec)))
	{
		if (pError != NULL)
		{
			/**pError = QString::fromStdString(XmlRpc::XmlRpcUtil::getLastErrorText());*/
			char* err;
			XmlRpc::XmlRpcUtil::error("%s", err);
			*pError = QString::fromStdString(std::string(err));
		}
		return RECORDERACCESS_RETVAL_NOCONN;
	}
	else if (client.isFault()) // if the client answers and says 'fault' (unknown command, bad parameters, ...)
	{
		std::string strResult("<unknown>");
		if (result["faultString"].getType() == XmlRpc::XmlRpcValue::TypeString)
		{
			strResult = std::string(result["faultString"]);
			if (pError != NULL)
			{
				*pError = QString::fromStdString(std::string(result["faultString"]));
			}
		}
		else
		{
			if (pError != NULL)
			{
				std::stringstream stream;
				stream << "Result: " << result;
				*pError = QString(stream.str().c_str());
			}
		}
		qDebug() << "client reports faultString=" << strResult.c_str();
		return RECORDERACCESS_RETVAL_NOSUPP;
	}
	else
	{
		return 0; // client gives a result
	}
}


/*! This function decodes run-length encoded recording information.
*  \param inData pointer to input data array
*  \param inSize size of input data array
*  \param outData pointer to returned decoded data array
*  \param outSize returned size of output array */
void RecorderAccess::runLengthDecode(char *inData, int inSize, char *outData, int &outSize)
{
	int  inIndex = 0;
	int  outIndex = 0;
	while (inIndex < inSize)
	{
		if ((inData[inIndex] & 0x80) && (inIndex < inSize - 1))
		{
			for (int i = 0; i < int(((unsigned char)inData[inIndex + 1])); i++)
			{
				outData[outIndex++] = inData[inIndex] & 0x7F;
			}
			inIndex += 2;
		}
		else
		{
			outData[outIndex++] = inData[inIndex++];
		}
	}

	outSize = outIndex;
}

// decodes value into utcMinutesOfDay, returns false if no data was available (not an error!),
// true if data has been decoded OK. Else, throws exceptions.
bool RecorderAccess::decodeRunLengthData(const XmlRpc::XmlRpcValue::BinaryData &data, char *utcMinutesOfDay)
{
	char compressedData[60 * 24];
	int inSize = int(data.size());

	// if data was returned
	if (inSize > 0)
	{
		if (data[0] != AD_RUN_LENGTH_ENCODING)
			qDebug() << "Different run length encoding as expected from recorder!" << " SOCKET protocol error";
		for (int j = 0; j < inSize - 1; j++)
			compressedData[j] = (char)data[j + 1];
		// decode run-length coded information
		int outSize;
		runLengthDecode(compressedData, inSize - 1, utcMinutesOfDay, outSize);
		if (outSize != 60 * 24)
			qDebug() << "Different data size as expected from recorder!" << " SOCKET protocol error";
		return true;
	}
	return false;
}

bool RecorderAccess::resultIsArray(
	const XmlRpc::XmlRpcClient &client,
	const XmlRpc::XmlRpcValue &result)
{
	return !client.isFault() && result.getType() == XmlRpc::XmlRpcValue::TypeArray;
}

bool RecorderAccess::resultIsBinary(
	const XmlRpc::XmlRpcClient &client,
	const XmlRpc::XmlRpcValue &result)
{
	return !client.isFault() && (result.getType() == XmlRpc::XmlRpcValue::TypePlainBinary || result.getType() == XmlRpc::XmlRpcValue::TypeBase64);
}

bool RecorderAccess::resultIsStruct(
	const XmlRpc::XmlRpcClient &client,
	const XmlRpc::XmlRpcValue &result)
{
	return !client.isFault() && result.getType() == XmlRpc::XmlRpcValue::TypeStruct;
}

int RecorderAccess::minutesOfDay(QDate date)
{
	QDateTime dateTime1 = QDateTime(date, QTime(0, 0));
	QDateTime dateTime2 = QDateTime(date.addDays(1), QTime(0, 0));

	return (dateTime1.secsTo(dateTime2) / 60);
}


