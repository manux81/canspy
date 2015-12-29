#ifndef QCANSOCKET_H
#define QCANSOCKET_H

#include "canbus/can_state.h"

#include <QAbstractSocket>
#include <QString>
#include <stdint.h>

class QCanSocket : public QAbstractSocket
{
	Q_OBJECT
	Q_PROPERTY(SocketState state READ state)
public:
	explicit QCanSocket(QString &dev, QObject *parent = 0);
	explicit QCanSocket(const char *dev, QObject *parent = 0);
	~QCanSocket();

	bool isSequential();

	int connect();
	int disconnect(void);

	int setBitrate(unsigned bitrate);
	int start();
	int stop();

	int getCanBusState(qcan_state_t *status);

	size_t send(unsigned id, uint8_t dlc, void *data);
	size_t recv(unsigned *id, uint8_t *dlc, void *data);

	SocketState state() const;

protected:
	/* Check the current can bus state and performs a reset if needed */
	int checkCurrentCanBusState(void);

signals:

public slots:

private:
	int skt;
	SocketState status;

	QString m_dev;
};

#endif // QCANSOCKET_H
