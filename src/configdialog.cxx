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


#include "configdialog.h"
#include "qappsettings.h"
#include <mainwindow.h>
#include <QFileDialog>

QCanalyzerConfigDialog::QCanalyzerConfigDialog(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	setFixedSize(size());
	listWidget->setCurrentRow(0);
	stackedWidget->setCurrentIndex(0);

	connect(listWidget, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
	        this, SLOT(changePage(QListWidgetItem *, QListWidgetItem*)));

	connect(interfaceComboBox, SIGNAL(activated(int)),
	        interfaceStackedWidget, SLOT(setCurrentIndex(int)));

	connect(this, SIGNAL(accepted()),
	        this, SLOT(acceptDriver()));

	connect(butOpenSimulation, SIGNAL(clicked()),
	        this, SLOT(selectSimulationFile()));

	connect(butOpenConfiguration, SIGNAL(clicked()),
	        this, SLOT(selectConfigurationFile()));

	setAttribute(Qt::WA_DeleteOnClose);

}

QCanalyzerConfigDialog::~QCanalyzerConfigDialog()
{
}

void QCanalyzerConfigDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
	if (!current) {
		current = previous;
	}
	stackedWidget->setCurrentIndex(listWidget->row(current));

}

void QCanalyzerConfigDialog::saveSettings()
{
	QString strValue;
	MainWindow *parentWindow = qobject_cast<MainWindow*>(parent());
	QAppSettings *settings = parentWindow->getSettings();

	settings->beginGroup("CANConnection");
	if(!edi8DevicesName->text().isEmpty() && !edi8DevicesBitRate->text().isEmpty()) {
		strValue = edi8DevicesName->text().trimmed();
		settings->setValue("8DevicesName", strValue);
		strValue = edi8DevicesBitRate->text().trimmed();
		settings->setValue("8DevicesBitRate", strValue);
	} else {
#if __linux
		settings->setValue("8DevicesName", "can0");
#else
		settings->setValue("8DevicesName", "ED00002");
#endif
		settings->setValue("8DevicesBitRate", "250000");
	}

	// for connection to ixxat
	if (!ixxatBitRateLineEdit->text().isEmpty()) {
		strValue = ixxatBitRateLineEdit->text().trimmed();
		settings->setValue("ixxatBitRate", strValue);
	} else {
		settings->setValue("ixxatBitRate", "250000");
	}
	// for connection to net
	if (!canNetServerIPLineEdit->text().isEmpty()) {
		strValue = canNetServerIPLineEdit->text().trimmed();
		settings->setValue("canNetServerIP", strValue);
	} else {
		settings->setValue("canNetServerIP", "127.0.0.1");
	}
	if (!canNetServerPortLineEdit->text().isEmpty()) {
		strValue = canNetServerPortLineEdit->text().trimmed();
		settings->setValue("canNetServerPort", strValue);
	} else {
		settings->setValue("canNetServerPort", "8888");
	}
	// for connection to PCAN-USB
	if(!ediPCANName->text().isEmpty() && !ediPCANBitRate->text().isEmpty()) {
		strValue = ediPCANName->text().trimmed();
		settings->setValue("PCANName", strValue);
		strValue = ediPCANBitRate->text();
		settings->setValue("PCANBitRate", strValue);
	} else {
		settings->setValue("PCANName", "can0");
		settings->setValue("PCANBitRate", "250000");
	}
	// for connection to Simulation
	if(!ediSimulationName->text().isEmpty()) {
		strValue = ediSimulationName->text().trimmed();
		settings->setValue("SimulationName", strValue);
	} else {
		settings->setValue("SimulationName", "");
	}

	settings->endGroup();

	settings->beginGroup("Logging");
	if(!ediSimulationName->text().isEmpty()) {
		strValue = ediConfigurationName->text().trimmed();
		settings->setValue("ConfigurationName", strValue);
	} else {
		settings->setValue("ConfigurationName", "");
	}
	if(logFileCheckBox->isChecked()) {
		if(!logFileLineEdit->text().isEmpty()) {
			strValue = logFileLineEdit->text().trimmed();
			settings->setValue("LogFile", logFileLineEdit->text());
			parentWindow->setLogToFileEnabled(true);
		} else {
			settings->setValue("LogFile", "message.tlog");
			parentWindow->setLogToFileEnabled(true);
		}
	} else {
		settings->setValue("LogFile", "no");
		parentWindow->setLogToFileEnabled(false);
	}
	settings->endGroup();
}

void QCanalyzerConfigDialog::on_logFileCheckBox_stateChanged()
{
	if(logFileCheckBox->isChecked()) {
		qDebug("ENABLE LINE EDIT");
		logFileLineEdit->setEnabled(true);
	} else {
		qDebug("DISABLE LINE EDIT");
		logFileLineEdit->setEnabled(false);
	}
}


void QCanalyzerConfigDialog::acceptDriver()
{
	MainWindow *parentWindow = qobject_cast<MainWindow*>(parent());
	QAppSettings *settings = parentWindow->getSettings();
	settings->setValue("CANConnection/actualConnection",
	                   interfaceComboBox->currentIndex());
	saveSettings();
}

void QCanalyzerConfigDialog::selectSimulationFile()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Open Log",".",
	                   "Logs (*.tlog *.asc)");
	if (! fileName.isEmpty())
		ediSimulationName->setText(fileName);
}

void QCanalyzerConfigDialog::selectConfigurationFile()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Open Configuration",".",
	                   "config (*.xml)");
	if (! fileName.isEmpty())
		ediConfigurationName->setText(fileName);
}
