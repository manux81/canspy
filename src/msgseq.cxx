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

#include "msgseq.h"
#include "ui_msgseq.h"
#include "utils.h"

#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QDebug>
#include <QDateTime>


MsgSeq::MsgSeq(QMainWindow *parent, QCanMonitor *monitor, QCanSendThread *send) :
	QMainWindow(parent),
	ui(new Ui::MsgSeq)
{
	QHeaderView *hdr;

	ui->setupUi(this);
	m_monitor = monitor;
	m_send = send;
	m_logmodel = new logModel(this);
	ui->msgTableView->setModel(m_logmodel);
	hdr = ui->msgTableView->horizontalHeader();
	hdr->setSectionResizeMode(0, QHeaderView::Fixed);
	hdr->setSectionResizeMode(1, QHeaderView::Fixed);
	hdr->setSectionResizeMode(2, QHeaderView::Fixed);
	hdr->setSectionResizeMode(3, QHeaderView::Fixed);
	hdr->setSectionResizeMode(4, QHeaderView::Fixed);
	hdr->setSectionResizeMode(5, QHeaderView::Stretch);
	hdr->setSectionResizeMode(6, QHeaderView::Fixed);
	ui->msgTableView->setColumnWidth(0, 60);
	ui->msgTableView->setColumnWidth(1, 60);
	ui->msgTableView->setColumnWidth(3, 60);
	ui->msgTableView->setColumnWidth(4, 30);
	ui->msgTableView->setColumnWidth(6, 30);
	ui->msgTableView->horizontalHeader()->setHighlightSections(false);
	ui->msgTableView->verticalHeader()->setDefaultSectionSize(17);
	ui->msgTableView->setAlternatingRowColors(true);
	connect(this, SIGNAL(msgEnqueue(can_packet_t)),
	        m_logmodel, SLOT(messageEnqueued(can_packet_t)));
	connect(ui->addMsgButton, SIGNAL(clicked()),
	        this, SLOT(addMsgButton_clicked()));
	connect(ui->removeAllButton, SIGNAL(clicked()),
	        this, SLOT(removeAllButton_clicked()));
	connect(ui->removeSelectedButton, SIGNAL(clicked()),
	        this, SLOT(removeSelectedButton_clicked()));
	connect(ui->sendSelectedButton, SIGNAL(clicked()),
	        this, SLOT(sendSelectedButton_clicked()));
	connect(ui->sendAllButton, SIGNAL(clicked()),
	        this, SLOT(sendAllButton_clicked()));
	connect(ui->openMsgSeq, SIGNAL(triggered()),
	        this, SLOT(openAction_triggered()));
}

MsgSeq::~MsgSeq()
{
	delete ui;
}

void MsgSeq::addMsgButton_clicked()
{
	can_packet_t packet;
	packet.id = 0x170;
	packet.dlc = 8;
	get_timestamp(&packet.tv_sec, &packet.tv_usec);
	memset(packet.data, 0xA, sizeof(char) * 8);
	emit msgEnqueue(packet);
}

void MsgSeq::removeAllButton_clicked()
{
	int rowCount = m_logmodel->rowCount();
	if (rowCount) {
		m_logmodel->removeContinousRows(0, rowCount);
	}
}

void MsgSeq::removeSelectedButton_clicked()
{
	QList<int> indexList;

	foreach(QModelIndex index, ui->msgTableView->selectionModel()->selectedRows()) {
		int row = index.row();
		indexList << row;
	}
	qSort(indexList.begin(), indexList.end(), qGreater<int>());

	for (int i = 0; i < indexList.size(); ++i) {
		m_logmodel->removeDisContinousRows(indexList[i], 1);
	}
}

void MsgSeq::rowToCanPacket(int row, can_packet_t &packet, uint64_t *msleep)
{
	QByteArray sendData;
	int len;

	for (int i = 0; i < m_logmodel->columnCount(); i++) {
		QModelIndex index2 = m_logmodel->index(row, i);
		QVariant field = m_logmodel->data(index2);
		switch (i) {
		case 0:
			packet.id = field.toString().toUInt(NULL, 16);
			break;

		case 3:
			if (msleep != NULL)
				*msleep = field.toUInt();
			break;

		case 4:
			packet.dlc = field.toUInt();
			break;

		case 5:
			sendData = QByteArray::fromHex(field.toString().toLatin1());
			len = qMin(sendData.size(), 8);
			memcpy(packet.data, (const unsigned char *)sendData.constData(), len);
			break;

		default:
			break;
		}
	}
}

void MsgSeq::loadFile(QString fileName)
{
	QFile file(fileName);
	can_packet_t packet;

	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		QMessageBox::warning(this, tr("CanSpy"),
		                     tr("Cannot read file %1:\n%2")
		                     .arg(fileName)
		                     .arg(file.errorString()));
		return;
	}
	QTextStream in(&file);
	while (!in.atEnd()) {
		QString line = in.readLine();
		packet = processLine(line);
		emit msgEnqueue(packet);
	}
}

can_packet_t MsgSeq::processLine(const QString &line)
{
	can_packet_t ret;
	unsigned col;

	QStringList list = line.split(" ", QString::SkipEmptyParts);
	col = 0;
	foreach (QString token, list) {
		if (col == 0)
			ret.id = token.toUInt(NULL, 16);
		else if (col == 1) {
			token.replace(QRegExp( "[" + QRegExp::escape( "[]" ) + "]" ), "");
			ret.dlc = token.toUInt();
		} else if (col < ret.dlc + 1U) {
			ret.data[col - 1] = token.toLongLong(NULL, 16);
		} else if (col == ret.dlc + 1U) {
			//TODO: Add support extended packet
		} else if (col == ret.dlc + 3U) {
			token = token.right(token.length() -2);
			QDateTime d =  QDateTime::fromString(token, QString("hh:mm:ss.zzz"));
			ret.tv_sec = d.toMSecsSinceEpoch() / 1000;
			ret.tv_usec = (d.toMSecsSinceEpoch() % 1000) * 1000.0;
		}
		col++;
	}

	return ret;
}

void MsgSeq::sendSelectedButton_clicked()
{
	can_packet_t packet;
	uint64_t ms_delay;
	uint64_t rowCount;
	uint64_t startRow;

	rowCount = ui->msgTableView->selectionModel()->selectedRows().count();
	startRow = ui->msgTableView->selectionModel()->selectedRows().first().row();
	foreach(QModelIndex index, ui->msgTableView->selectionModel()->selectedRows()) {
		ms_delay = 100U;
		rowToCanPacket(index.row(), packet, &ms_delay);
		ui->statusbar->showMessage(QString("send %1 of %2...")
		                           .arg(index.row() - startRow + 1)
		                           .arg(rowCount));
		m_send->sendPacket(packet);
		QThread::msleep(ms_delay);
	}
}

void MsgSeq::sendAllButton_clicked()
{
	can_packet_t packet;
	uint64_t ms_delay;
	uint64_t rowCount;

	rowCount = m_logmodel->rowCount();
	for (uint64_t row = 0; row < rowCount; row++) {
		ms_delay = 100U;
		rowToCanPacket(row, packet, &ms_delay);
		ui->statusbar->showMessage(QString("send %1 of %2...")
		                           .arg(row + 1)
		                           .arg(rowCount));
		m_send->sendPacket(packet);
		QThread::msleep(ms_delay);
	}
}

void MsgSeq::openAction_triggered()
{
	int rows;
	QString fileNamePath = QFileDialog::getOpenFileName(this,
	                       tr("Open sequence file"), ".", tr("Text log(*.tlog)"));
	if ((rows = m_logmodel->rowCount())) {
		m_logmodel->removeContinousRows(0, rows);
	}
	loadFile(fileNamePath);
}

