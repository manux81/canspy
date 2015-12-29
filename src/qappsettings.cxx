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


#include "qappsettings.h"
#include <QSettings>
#include <QFile>
#include <QMessageBox>
#include <QDir>

#define DEFAULT_NAME "./setting.txt"

QAppSettings::QAppSettings(QObject *parent)
	: QSettings(DEFAULT_NAME, QSettings::IniFormat, parent)
{
	qDebug("Printing default settings!");

	beginGroup("CANConnection");

	if(!contains("actualConnection"))
		setValue("actualConnection", EDEVICES_USB);

	switch (value("actualConnection").toInt()) {

	case EDEVICES_USB:
		qDebug("Using connection over 8Devices usb.");
		break;
	case IXXAT_USB:
		qDebug("Using connection over IXXAT usb.");
		break;
	case NETCAN:
		qDebug("Using connection over TCP.");
		break;
	case PCAN_USB:
		qDebug("Using connection over PCAN usb.");
		break;
	case SIMULATION:
		qDebug("Using connection over Simulation.");
		break;
	default:
		qDebug("No default connection. Please choose connection which you want to use.");

	}

	if(contains("8DevicesName")) {
		qDebug("%s",qPrintable(value("8DevicesName").toString()));
	} else {
		setValue("8DevicesName", "ED000200");
	}

	if(contains("8DevicesBitRate")) {
		qDebug("%s",qPrintable(value("8DevicesBitRate").toString()));
	} else {
		setValue("8DevicesBitRate", "250000");
	}

	if(contains("ixxatBitRate")) {
		qDebug("%s",qPrintable(value("ixxatBitRate").toString()));
	} else {
		setValue("ixxatBitRate", "250000");
	}

	if(contains("canDevice")) {
		qDebug("%s",qPrintable(value("canDevice").toString()));
	} else {
		setValue("canDevice", "no");
	}

	if(contains("canNetServerIP")) {
		qDebug("%s",qPrintable(value("canNetServerIP").toString()));
	} else {
		setValue("canNetServerIP", "127.0.0.1");
	}
	if(contains("canNetServerPort")) {
		qDebug("%s",qPrintable(value("canNetServerPort").toString()));
	} else {
		setValue("canNetServerPort", "8888");
	}

	endGroup();

	beginGroup("Logging");
	if(contains("LogFile")) {
		qDebug("%s",qPrintable(value("LogFile").toString()));
	} else {
		setValue("LogFile", "no");
	}
	endGroup();

	beginGroup("Paths");
	if(contains("defaultOpenFilePath")) {
		qDebug("%s",qPrintable(value("defaultOpenFilePath").toString()));
	} else {
		setValue("defaultOpenFilePath", QDir::currentPath());
	}

	if(contains("defaultSaveFilePath")) {
		qDebug("%s",qPrintable(value("defaultSaveFilePath").toString()));
	} else {
		setValue("defaultSaveFilePath", QDir::currentPath());
	}
	endGroup();
}

QAppSettings::QAppSettings(QString settingsFileName, QObject *parent)
	: QSettings(settingsFileName, QSettings::NativeFormat, parent)
{
}
