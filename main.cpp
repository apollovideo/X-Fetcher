#include <QtCore/QCoreApplication>
#include <QDebug>

#include "ContactEvents.h"


int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	ServiceSettings* pServiceSettings = new ServiceSettings(QThread::currentThread());

	ContactEvents* eventHandler = new ContactEvents(pServiceSettings, QThread::currentThread());

	eventHandler->handleContactEvents();

	return a.exec();
}
