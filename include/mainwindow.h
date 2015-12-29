/*
 *  canspy - A simple tool for users who need to interface with a device based on
 *           CAN (CAN/CANopen/J1939/NMEA2000/DeviceNet) such as motors,
 *           sensors and many other devices.
 *  Copyright (C) 2015-2016  Manuele Conti (manuele.conti@gmail.com)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * This code is made available on the understanding that it will not be
 * used in safety-critical situations without a full and competent review.
 */


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QProgressBar>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMap>
#include <QTime>


#include "qcanrecvthread.h"
#include "qcansendthread.h"
#include "qcansocket.h"
#include "qcanmonitor.h"
#include "qappsettings.h"
#include "qdelegatecolor.h"
#include "logmodel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

	typedef struct {
		int64_t lastSec;
		int64_t lastUSec;
		quint64 row;
		quint64 count;
		quint8 data[8];
	} statistic_t;

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	QAppSettings *getSettings(void);
	void setLogToFileEnabled(bool);
	bool saveFileStandard(const QString &);
	bool saveFileAsc(const QString &);
	virtual bool eventFilter(QObject *, QEvent *);

private slots:
	void showPacket(can_packet_t);
	void connectToDevice(void);
	void disconnectFromDevice(void);
	void about(void);
	void quit(void);
	void sendPacket(void);
	void updateStatus(void);
	void editOptions(void);
	void filterIdChanged(QString);
	bool saveLogFile(void);
	void clearLogs(void);
	void clearAll(void);
	void enableLogChanged(bool);
	void enableSoundChanged(bool);
	void enableHexChanged(bool);
	void sendCycle(void);
	void cycleTimeChanged(QString);
	void exportToCSV(void);
	void showSendSequenceDialog(void);

private:
	void initActionsConnections(void);

	Ui::MainWindow *ui;
	QCanRecvThread *m_recvthr;
	QCanSendThread *m_sendthr;
	QCanSocket *m_sk;
	QTimer *m_timer;
	QTimer *m_timer_cycle;
	QLabel *m_labPacketRecv;
	QLabel *m_labPacketSend;
	QLabel *m_labNumberPDO;
	QLabel *m_labConfig;
	quint64 m_pkg_recv;
	quint64 m_pkg_send;
	quint64 m_bit_recv;
	quint64 m_bitrate;
	quint16 m_percent;
	quint16 m_cycletime;
	bool m_logToFile;
	QAppSettings *m_appSettings;
	logModel *m_model_log;
	QStandardItemModel *m_model_stat;
	QSortFilterProxyModel *m_model_sort_stat;
	QMap<unsigned, statistic_t *> m_stats;
	QProgressBar *m_busload;
	bool m_sound;
	QCanMonitor *m_monitor;
};


#endif // MAINWINDOW_H
