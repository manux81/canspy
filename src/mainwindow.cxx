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


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "configdialog.h"
#include "canbus/can_drv.h"
#include "drivers/can_socket_ops.h"
#include "drivers/net_ops.h"
#include "msgseq.h"
#include "utils.h"


#include <QDebug>
#include <QMessageBox>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QMapIterator>
#include <QInputDialog>


MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	QHeaderView *hdr;

	ui->setupUi(this);
	this->setFixedSize(this->size());
	m_labPacketRecv = new QLabel("RECV: 0", this);
	m_labPacketSend = new QLabel("SENT: 0", this);
	m_labNumberPDO = new QLabel("PDO: 0", this);
	m_labConfig = new QLabel("", this);
	m_model_log = new logModel(NULL);
	m_busload = new QProgressBar(this);
	m_busload->setFixedWidth(100);

	ui->msgLog->setModel(m_model_log);
	ui->msgLog->setAlternatingRowColors(true);
	ui->msgLog->horizontalHeader()->setStretchLastSection(false);

	ui->msgLog->verticalHeader()->hide();
	hdr = ui->msgLog->horizontalHeader();
	hdr->setSectionResizeMode(0, QHeaderView::Fixed);
	hdr->setSectionResizeMode(1, QHeaderView::Fixed);
	hdr->setSectionResizeMode(2, QHeaderView::Fixed);
	hdr->setSectionResizeMode(3, QHeaderView::Fixed);
	hdr->setSectionResizeMode(4, QHeaderView::Fixed);
	hdr->setSectionResizeMode(5, QHeaderView::Stretch);
	hdr->setSectionResizeMode(6, QHeaderView::Fixed);
	ui->msgLog->setColumnWidth(0, 60);
	ui->msgLog->setColumnWidth(1, 60);
	ui->msgLog->setColumnWidth(3, 60);
	ui->msgLog->setColumnWidth(4, 30);
	ui->msgLog->setColumnWidth(6, 30);
	ui->msgLog->verticalHeader()->setDefaultSectionSize(17);
	ui->msgLog->horizontalHeader()->setHighlightSections(false);
	ui->statusBar->addPermanentWidget(m_labConfig);
	ui->statusBar->addPermanentWidget(m_busload);
	ui->statusBar->addPermanentWidget(m_labPacketSend);
	ui->statusBar->addPermanentWidget(m_labPacketRecv);
	ui->statusBar->addPermanentWidget(m_labNumberPDO);

	m_model_stat = new QStandardItemModel(this);
	m_model_stat->setHorizontalHeaderLabels(QStringList() << "ID" << "Count"
											<< "Elapsed"<< "DATA");
	m_model_sort_stat = new QSortFilterProxyModel(this);
	m_model_sort_stat->setSourceModel(m_model_stat);
	m_model_sort_stat->sort(0);
	QDelegateColor *delegate = new QDelegateColor;
	ui->msgStats->setItemDelegate(delegate);
	ui->msgStats->setModel(m_model_sort_stat);
	ui->msgStats->setAlternatingRowColors(true);

	ui->msgStats->verticalHeader()->hide();

	ui->msgStats->verticalHeader()->setDefaultSectionSize(17);
	hdr = ui->msgStats->horizontalHeader();
	hdr->setSectionResizeMode(0, QHeaderView::Fixed);
	hdr->setSectionResizeMode(1, QHeaderView::Fixed);
	hdr->setSectionResizeMode(2, QHeaderView::Fixed);
	hdr->setSectionResizeMode(3, QHeaderView::Stretch);
	ui->msgStats->setColumnWidth(0, 60);
	ui->msgStats->setColumnWidth(1, 60);
	ui->msgStats->horizontalHeader()->setHighlightSections(false);

	setWindowTitle("CanSpy " VERSION);
	qRegisterMetaType<can_packet_t>();
	m_recvthr = NULL;
	m_sk = NULL;
	m_pkg_recv = 0;
	m_pkg_send = 0;
	m_bit_recv = 0;
	m_bitrate  = 0;
	m_percent = 0;
	m_cycletime = 0;
	m_sound = false;
	m_appSettings = new QAppSettings(this);
	m_timer = new QTimer();
	m_timer->start(1000);
	m_timer_cycle = new QTimer();
	m_timer_cycle->setTimerType(Qt::PreciseTimer);
	initActionsConnections();
	this->installEventFilter(this);
}

MainWindow::~MainWindow()
{
	delete ui;
}

QAppSettings *MainWindow::getSettings()
{
	return m_appSettings;
}

void MainWindow::setLogToFileEnabled(bool value)
{
	m_logToFile = value;
}

void MainWindow::showPacket(can_packet_t packet)
{
	QString temp;
	static QTime curTime;
	quint32 pck_id;
	QStandardItem * it;
	QList <QStandardItem *> listItems;

	m_pkg_recv++;
	if (m_sound && ! (m_pkg_recv % 100))
		QApplication::beep();

	pck_id = packet.id & EFF_MASK;
	if (packet.id & EFF_FLAG)
		m_bit_recv += (389 + packet.dlc*48)/5;
	else
		m_bit_recv += (269 + packet.dlc*48)/5;
	m_labPacketRecv->setText(QString("RECV:%1").arg(m_pkg_recv));

	curTime = ((QDateTime::fromTime_t(packet.tv_sec)).time());
	curTime = curTime.addMSecs(packet.tv_usec/1000);

	if (! m_stats.contains(pck_id)) {
		listItems.clear();
		statistic_t *s = new statistic_t;
		s->lastSec = packet.tv_sec;
		s->lastUSec = packet.tv_usec;
		memset(s->data, 0, 8);
		s->count = 0;
		s->row = m_model_stat->rowCount();
		it = new QStandardItem(QString::number(pck_id, 16).toUpper());
		it->setEditable(false);
		listItems.push_back(it);
		it = new QStandardItem("0");
		it->setEditable(false);
		listItems.push_back(it);
		it = new QStandardItem("0");
		it->setEditable(false);
		listItems.push_back(it);
		it = new QStandardItem("");
		it->setEditable(false);
		listItems.push_back(it);
		m_stats[pck_id] = s;
		m_model_stat->appendRow(listItems);
	}
	temp = "";
	for (unsigned i = 0; i < packet.dlc; i++) {
		if (m_stats[pck_id]->count <= 1 || packet.data[i] != m_stats[pck_id]->data[i])
			temp += "<font color=\"red\">";
		else
			temp += "<font color=\"black\">";
		QString tok;
		temp += tok.sprintf("%02X ",packet.data[i]).toUpper();

		if (packet.data[i] != m_stats[pck_id]->data[i])
			temp += "</font>";
		m_stats[pck_id]->data[i] = packet.data[i];
	}
	it = m_model_stat->item(m_stats[pck_id]->row,1);
	it->setText(QString::number(++m_stats[pck_id]->count));
	it = m_model_stat->item(m_stats[pck_id]->row,3);
	it->setText(temp);
	it = m_model_stat->item(m_stats[pck_id]->row,2);
	int64_t msec = (((packet.tv_sec - m_stats[pck_id]->lastSec) * 1000000) +
					packet.tv_usec - m_stats[pck_id]->lastUSec)/ 1000;
	it->setText(QString::number(msec));
	m_stats[pck_id]->lastSec  = packet.tv_sec;
	m_stats[pck_id]->lastUSec  = packet.tv_usec;
	m_labNumberPDO->setText(QString("PDO:%1").arg(m_stats.size()));
}

void MainWindow::connectToDevice(void)
{
	QString deviceName;
	int actualConntection;
	uint16_t val16;
	QString temp;

	if (m_sk != NULL)
		return;

	m_appSettings->beginGroup("CANConnection");
	actualConntection = m_appSettings->value("actualConnection").toInt();
	switch (actualConntection) {
	case 0:
		can_ops = get_can_ops("USB2CAN 8Devices");
		deviceName = m_appSettings->value("8DevicesName").toString();
		m_bitrate = m_appSettings->value("8DevicesBitRate").toUInt();
		m_labConfig->setText(QString("[%1, %2 kbit/s]:").arg(deviceName).arg(m_bitrate / 1000));
		break;

	case 1:
		can_ops = get_can_ops("IXXAT USB");
		deviceName = "ixxat";
		m_bitrate = m_appSettings->value("ixxatBitRate").toUInt();
		m_labConfig->setText(QString("[%1, %2 kbit/s]:").arg(deviceName).arg(m_bitrate / 1000));
		break;

	case 2:
		can_ops = get_can_ops("CAN Over TCP");
		deviceName = "TCP";
		m_bitrate = 0;
		temp = m_appSettings->value("canNetServerIP").toString();
		can_ops->attribute_set(NET_SOCKET_ADDR,
							   temp.toLatin1().data(), temp.length());
		val16 = m_appSettings->value("canNetServerPort").toUInt();
		can_ops->attribute_set(NET_SOCKET_PORT,
							   &val16, sizeof(uint16_t));
		m_labConfig->setText(QString("[%1, %2:%3]:").arg(deviceName).arg(temp).arg(val16));
		break;

	case 3:
		can_ops = get_can_ops("PCAN-USB");
		deviceName = m_appSettings->value("PCANName").toString();
		m_bitrate = m_appSettings->value("PCANBitRate").toUInt();
		m_labConfig->setText(QString("[%1, %2 kbit/s]:").arg(deviceName).arg(m_bitrate / 1000));
		break;

	case 4:
		can_ops = get_can_ops("Simulation");
		deviceName = m_appSettings->value("SimulationName").toString();
		m_bitrate = 0;
		m_labConfig->setText(QString("[%1]:").arg(deviceName));
		break;


	default:
		break;
	}

	if (NULL == can_ops) {
		qDebug() << "Connection not supported";
		return;
	}
	m_appSettings->endGroup();
	m_monitor = new QCanMonitor(this);
	m_sk = new QCanSocket(deviceName, m_bitrate);
	if (m_sk->connect() <= 0) {
		delete m_sk;
		m_sk = NULL;
		QMessageBox::warning(this, tr("Connection warning..."),
							 tr("Device is not present."),
							 QMessageBox::Ok);
		return;
	}

	if (m_sk->start() < 0) {
		delete m_sk;
		m_sk = NULL;
		QMessageBox::warning(this, tr("Connection error..."),
							 tr("Device is not working."),
							 QMessageBox::Ok);
		return;
	}
	m_recvthr = new QCanRecvThread(m_sk);
	m_recvthr->linkPacketConsumer(m_monitor);
	m_monitor->setFilterId(ui->ediFilterId->text());
	connect(m_monitor, SIGNAL(packetReceived(can_packet_t)),
			m_model_log, SLOT(messageEnqueued(can_packet_t)));

	connect(m_monitor, SIGNAL(packetReceived(can_packet_t)),
			this, SLOT(showPacket(can_packet_t)));
	m_sendthr = new QCanSendThread(m_sk);
	m_recvthr->start();
	m_recvthr->setPriority(QThread::HighestPriority);
}

void MainWindow::disconnectFromDevice()
{
	if (m_sk == NULL)
		return;

	m_recvthr->unlinkPacketConsumer(m_monitor);
	disconnect(m_monitor);
	disconnect(m_sendthr);
	disconnect(m_recvthr);

	m_recvthr->stop();
	m_recvthr->terminate();
	m_recvthr->wait();

	m_sk->disconnect();
	m_sk->close();

	delete m_monitor;
	delete m_recvthr;
	delete m_sendthr;
	delete m_sk;

	m_monitor = NULL;
	m_sendthr = NULL;
	m_recvthr = NULL;
	m_sk = NULL;
	m_model_log->moveToThread(this->thread());
	m_pkg_recv = 0;
	m_pkg_send = 0;
	m_bit_recv = 0;
	m_percent = 0;
}

void MainWindow::about(void)
{
	QMessageBox msg(tr("About CanSpy"),
					tr("<b>CanSpy " VERSION
					   "</b><br><br>Copyright 2015-2016 Manuele Conti (manuele.conti@gmail.com) "
					   "all rights reserved.<br><br><br>"
					   "The program is provided AS IS with NO WARRANTY OF ANY KIND,<br>"
					   "INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY<br>"
					   "AND FITNESS FOR A PARTICULAR PURPOSE.<br><br>"
					   ), QMessageBox::Information, 0, 0, 0, this);

	QPixmap pm(QLatin1String(":icon.ico"));
	if (!pm.isNull())
		msg.setIconPixmap(pm);
	msg.exec();
}

void MainWindow::quit()
{
	disconnectFromDevice();
}

void MainWindow::sendPacket()
{
	can_packet_t packet;

	if (m_sk == NULL ||
			ui->ediID->text().isEmpty() ||
			ui->ediDlc->text().isEmpty())
		return;

	packet.id = ui->ediID->text().toInt(0, 16);
	packet.dlc = ui->ediDlc->text().toInt();
	packet.data[0] = ui->ediByte0->text().toUInt(NULL, 16);
	packet.data[1] = ui->ediByte1->text().toUInt(NULL, 16);
	packet.data[2] = ui->ediByte2->text().toUInt(NULL, 16);
	packet.data[3] = ui->ediByte3->text().toUInt(NULL, 16);
	packet.data[4] = ui->ediByte4->text().toUInt(NULL, 16);
	packet.data[5] = ui->ediByte5->text().toUInt(NULL, 16);
	packet.data[6] = ui->ediByte6->text().toUInt(NULL, 16);
	packet.data[7] = ui->ediByte7->text().toUInt(NULL, 16);
	packet.direction = DIRECTION_TX;
	if (ui->rtrCheckBox->isChecked()) {
		packet.id |= RTR_FLAG;
	}

	if (ui->extCheckBox->isChecked()) {
		packet.id |= EFF_FLAG;
	}
	m_sendthr->sendPacket(packet);
	get_timestamp(&packet.tv_sec, &packet.tv_usec);
	m_model_log->messageEnqueued(packet);
	showPacket(packet);

	m_pkg_send++;
	m_labPacketSend->setText(QString("SENT:%1").arg(m_pkg_send));
}

void MainWindow::updateStatus()
{
	qcan_state_t state;

	m_busload->setValue(m_percent);
	if (m_sk == NULL) {
		ui->statusBar->showMessage("Disconnected");
		return;
	}
	if (m_sk->getCanBusState(&state) != 0) {
		disconnectFromDevice();
	}
	switch (state) {
	case QCAN_STATE_ACTIVE:
		ui->statusBar->showMessage("Active");
		break;

	case QCAN_STATE_WARNING:
		ui->statusBar->showMessage("Warning");
		break;

	case QCAN_STATE_PASSIVE:
		ui->statusBar->showMessage("Passive");
		break;

	case QCAN_STATE_BUS_OFF:
		ui->statusBar->showMessage("Bus OFF");
		break;

	case QCAN_STATE_STOPPED:
		ui->statusBar->showMessage("Stopped");
		break;

	case QCAN_STATE_SLEEPING:
		ui->statusBar->showMessage("Sleeping");
		break;

	case QCAN_STATE_UNKNOWN:
		ui->statusBar->showMessage("Unknown");
		break;

	default:
		break;
	}

	if (m_bitrate)
		m_percent =	(m_bit_recv *100)/ m_bitrate;
	else
		m_percent = 0;
	m_bit_recv = 0;
}

void MainWindow::editOptions()
{
	QCanalyzerConfigDialog *openConfig = new QCanalyzerConfigDialog(this);

	m_appSettings->beginGroup("CANConnection");
	openConfig->edi8DevicesName->setText(m_appSettings->value("8DevicesName").toString());
	openConfig->edi8DevicesBitRate->setText(m_appSettings->value("8DevicesBitRate").toString());
	openConfig->ediPCANName->setText(m_appSettings->value("PCANName").toString());
	openConfig->ediPCANBitRate->setText(m_appSettings->value("PCANBitRate").toString());
	openConfig->canNetServerIPLineEdit->setText(m_appSettings->value("canNetServerIP").toString());
	openConfig->canNetServerPortLineEdit->setText(m_appSettings->value("canNetServerPort").toString());
	openConfig->ixxatBitRateLineEdit->setText(m_appSettings->value("ixxatBitRate").toString());
	openConfig->ediSimulationName->setText(m_appSettings->value("SimulationName").toString());

	//It sets the chosen item in the configuration dialog based on actual connection.
	openConfig->interfaceComboBox->setCurrentIndex(m_appSettings->value("actualConnection").toInt());
	openConfig->interfaceStackedWidget->setCurrentIndex(m_appSettings->value("actualConnection").toInt());

	openConfig->labFileName->setText(m_appSettings->fileName());
	m_appSettings->endGroup();

	m_appSettings->beginGroup("Logging");
	openConfig->ediConfigurationName
			->setText(m_appSettings->value("ConfigurationName").toString());
	if(m_appSettings->value("LogFile").toString()=="no") {
		openConfig->logFileCheckBox->setCheckState(Qt::Unchecked);
		openConfig->logFileLineEdit->setDisabled(true);
	} else {
		openConfig->logFileCheckBox->setCheckState(Qt::Checked);
		openConfig->logFileLineEdit->setText(m_appSettings->value("LogFile").toString());
	}
	m_appSettings->endGroup();

	openConfig->exec();
}

void MainWindow::filterIdChanged(QString text)
{
	if (m_monitor != NULL)
		m_monitor->setFilterId(text);
}

bool MainWindow::saveLogFile()
{
	QString fileName;
	QString selectedFilter;
	int type;

	if (!m_model_log->rowCount()) {
		QMessageBox::warning(this, tr("CanSpy"),
							 tr("Log is empty!"),
							 QMessageBox::Ok);

		return false;
	} else {
		fileName = QFileDialog::getSaveFileName(this, tr("Save MsgLog File"),
												m_appSettings->value("defaultSaveFilePath").toString() +
												"/untitled.tlog", tr("Text LogFiles(*.tlog);;Vector Asc(*.asc)"),
												&selectedFilter);
		if (fileName.isEmpty())
			return false;

		QFileInfo fi(fileName);
		m_appSettings->setValue("defaultSaveFilePath",
								fi.absoluteDir().absolutePath());

		if (selectedFilter.contains("tlog")) {
			if (!fileName.contains("tlog"))
				fileName.append(".tlog");
			type = 0;
		}

		if (selectedFilter.contains("asc")) {
			if (!fileName.contains("asc"))
				fileName.append(".asc");
			type = 1;
		}

		if (selectedFilter.contains("Any")) {
			fileName.append(".tlog");
			type = 0;
		}
	}
	if (type == 0)
		return saveFileStandard(fileName);
	else
		return saveFileAsc(fileName);
}

void MainWindow::clearLogs(void)
{
	quint64 rows;

	rows = m_model_log->rowCount();
	m_model_log->removeRows(0, rows);
	m_pkg_recv = 0;
	m_pkg_send = 0;
	m_labPacketSend->setText(QString("SENT:%1").arg(m_pkg_send));
	m_labPacketRecv->setText(QString("RECV:%1").arg(m_pkg_recv));
	m_labNumberPDO->setText(QString("PDO:0"));
}

void MainWindow::clearAll(void)
{
	quint64 rows;

	rows = m_model_log->rowCount();
	m_model_log->removeRows(0, rows);
	rows = m_model_stat->rowCount();
	m_model_stat->removeRows(0, rows);
	m_pkg_recv = 0;
	m_pkg_send = 0;
	m_bit_recv = 0;
	m_percent = 0;
	QMapIterator <unsigned, statistic_t *>itr(m_stats);
	for (; itr.hasNext();) {
		itr.next();
		statistic_t *s = itr.value();
		delete s;
	}
	m_stats.clear();
	m_labPacketSend->setText(QString("SENT: %1").arg(m_pkg_send));
	m_labPacketRecv->setText(QString("RECV: %1").arg(m_pkg_recv));
	m_labNumberPDO->setText(QString("PDO: 0"));
}

void MainWindow::enableLogChanged(bool enable)
{
	if (m_model_log)
		m_model_log->setEnable(enable);
}

void MainWindow::enableSoundChanged(bool enable)
{
	m_sound = enable;
}

void MainWindow::enableHexChanged(bool enable)
{
	if (m_model_log)
		m_model_log->setHexLayout(enable);
}

void MainWindow::sendCycle()
{
	if (m_sendthr && ui->chkCycleTime->isChecked()) {
		sendPacket();
	}
}

void MainWindow::cycleTimeChanged(QString strValue)
{
	quint16 tmp;
	bool ok;

	tmp = strValue.toUInt(&ok);
	if (ok) {
		m_cycletime = tmp;
	}
	if (!ok || strValue.isEmpty()) {
		m_cycletime = 0;
	}
	m_timer_cycle->stop();
	if (m_cycletime > 0) {
		m_timer_cycle->start(m_cycletime);
	} else {
		m_timer_cycle->stop();
	}
}

void MainWindow::exportToCSV()
{
}

void MainWindow::showSendSequenceDialog()
{
	MsgSeq *msgSeqDialog;

	msgSeqDialog = new MsgSeq(this, m_monitor, m_sendthr);
	msgSeqDialog->setWindowModality(Qt::ApplicationModal);
	msgSeqDialog->show();
	delete msgSeqDialog;
}

void MainWindow::initActionsConnections(void)
{
	connect(ui->actionConnect, SIGNAL(triggered()),
			this, SLOT(connectToDevice()));
	connect(ui->actionDisconnect, SIGNAL(triggered()),
			this, SLOT(disconnectFromDevice()));
	connect(ui->actionAbout, SIGNAL(triggered()),
			this, SLOT(about()));
	connect(ui->actionExit, SIGNAL(triggered()),
			this, SLOT(close()));
	connect(qApp, SIGNAL(lastWindowClosed()),
			this, SLOT(quit()));
	connect(ui->butSend, SIGNAL(clicked()),
			this, SLOT(sendPacket()));
	connect(m_timer, SIGNAL(timeout()),
			this, SLOT(updateStatus()));
	connect(m_timer_cycle, SIGNAL(timeout()),
			this, SLOT(sendCycle()), Qt::QueuedConnection);
	connect(ui->actionOptions, SIGNAL(triggered()),
			this, SLOT(editOptions()));
	connect(ui->ediFilterId, SIGNAL(textChanged(QString)),
			this, SLOT(filterIdChanged(QString)));
	connect(ui->actionSave, SIGNAL(triggered()),
			this, SLOT(saveLogFile()));
	connect(ui->btnClearLog, SIGNAL(clicked()),
			this, SLOT(clearLogs()));
	connect(ui->btnClearAll, SIGNAL(clicked()),
			this, SLOT(clearAll()));
	connect(ui->chkEnableLog, SIGNAL(clicked(bool)),
			this, SLOT(enableLogChanged(bool)));
	connect(ui->chkEnableSound, SIGNAL(clicked(bool)),
			this, SLOT(enableSoundChanged(bool)));
	connect(ui->ediCycleTime, SIGNAL(textChanged(QString)),
			this, SLOT(cycleTimeChanged(QString)));
	connect(ui->actionExport, SIGNAL(triggered()),
			this, SLOT(exportToCSV()));
	connect(ui->actionSendSequence, SIGNAL(triggered()),
			this, SLOT(showSendSequenceDialog()));
	connect(ui->chkEnableHex, SIGNAL(clicked(bool)),
			this, SLOT(enableHexChanged(bool)));
}

bool MainWindow::saveFileStandard(const QString &fileNamePath)
{
	int rowCount;

	QFile file(fileNamePath);
	QFileInfo fi(fileNamePath);
	QApplication::setOverrideCursor(Qt::WaitCursor);
	if (!file.open(QFile::WriteOnly | QFile::Text)) {
		QMessageBox::warning(this, tr("Application"),
							 tr("Cannot write file %1:\n%2.")
							 .arg(fileNamePath)
							 .arg(file.errorString()));
		return false;
	}
	QTextStream out(&file);
	rowCount = m_model_log->rowCount();
	for(int row = 0; row < rowCount; row++) {
		QModelIndex index0 = m_model_log->index(row, 0);
		QModelIndex index1 = m_model_log->index(row, 1);
		QModelIndex index2 = m_model_log->index(row, 2);
		QModelIndex index3 = m_model_log->index(row, 3);
		QModelIndex index4 = m_model_log->index(row, 4);
		QModelIndex index5 = m_model_log->index(row, 5);
		QString s;
		out << index0.data().toString() << " ";
		out << "[" << index4.data().toString() << "] ";
		out << index5.data().toString() <<
			   s.fill(' ', 39 - index5.data().toString().length() + 5);
		out << index1.data().toString() << " T:" << index2.data().toString()
			<< " " << index3.data().toString() << endl;
	}
	QApplication::restoreOverrideCursor();
	QMessageBox::information(this, tr("CanSpy"),
							 tr("Log is done."),
							 QMessageBox::Ok);

	return true;
}

bool MainWindow::saveFileAsc(const QString &fileNamePath)
{
	int rowCount;

	QFile file(fileNamePath);
	QFileInfo fi(fileNamePath);
	QApplication::setOverrideCursor(Qt::WaitCursor);
	if (!file.open(QFile::WriteOnly | QFile::Text)) {
		QMessageBox::warning(this, tr("Application"),
							 tr("Cannot write file %1:\n%2.")
							 .arg(fileNamePath)
							 .arg(file.errorString()));
		return false;
	}
	QTextStream out(&file);
	out << "date Mon Jun 29 11:45:47.294 am 2015" << endl;
	out << "base hex  timestamps absolute" << endl;
	out << "internal events logged" << endl;
	out << "WriteLevel = 3" << endl;
	out << "// version 8.2.1" << endl ;
	rowCount = m_model_log->rowCount();
	for(int row = 0; row < rowCount; row++) {
		QModelIndex index0 = m_model_log->index(row, 0);
		QModelIndex index2 = m_model_log->index(row, 2);
		QModelIndex index4 = m_model_log->index(row, 4);
		QModelIndex index5 = m_model_log->index(row, 5);
		QModelIndex index6 = m_model_log->index(row, 6);
		out << "   ";
		out << index2.data().toString() << " 1  ";
		quint32 tmp = index0.data().toUInt();
		out << QString::number(tmp, 16) << "             ";
		out << index6.data().toString() << "   d ";
		out << index4.data().toString() << " ";
		out << index5.data().toString() << "  Length = 0 BitCount = 0 ID = ";
		tmp = index0.data().toString().toLongLong(NULL, 16);
		out << QString::number(tmp) << endl;
	}
	QApplication::restoreOverrideCursor();
	QMessageBox::information(this, tr("CanSpy"),
							 tr("Log is done."),
							 QMessageBox::Ok);

	return true;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::ContextMenu) {
		return true;
	}
	return QMainWindow::eventFilter(obj, event);
}
