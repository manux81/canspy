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



#include "trigger.h"
#include <mainwindow.h>

QTriggerDialog::QTriggerDialog(QWidget *parent)
	: QDialog(parent),
	ui(new Ui::triggerdialog)
{
	ui->setupUi(this);
	setFixedSize(size());
	ui->ediLevAIDEnd->setEnabled(false);
	ui->ediLevAValueEnd->setEnabled(false);
	ui->ediLevBIDEnd->setEnabled(false);
	ui->ediLevBValueEnd->setEnabled(false);
	ui->ediActionCommand->setVisible(false);
	m_level_condition = 0;
	m_action_delay_enable = 0;
	m_action_delay_value = 0;
	m_action_code = 0;
	m_action_cmd = "";

	m_level_a_count_limit = 1;
	m_level_a_check_value_enable = false;
	m_level_a_check_data = 0;
	m_level_a_value_condition = 0;
	m_level_a_value_start = 0;
	m_level_a_value_end = 0;
	m_level_a_check_id_enable = false;
	m_level_a_id_condition = 0;
	m_level_a_id_start = 0;
	m_level_a_id_end = 0;
	m_level_b_count_limit = 1;
	m_level_b_check_value_enable = false;
	m_level_b_check_data = 0;
	m_level_b_value_condition = 0;
	m_level_b_value_start = 0;
	m_level_b_value_end = 0;
	m_level_b_check_id_enable = false;
	m_level_b_id_condition = 0;
	m_level_b_id_start = 0;
	m_level_b_id_end = 0;


	initActionsConnections();
}

int QTriggerDialog::levelCondition() const
{
	return m_level_condition;
}

int QTriggerDialog::action() const
{
	return m_action_code;
}

bool QTriggerDialog::actionDelay() const
{
	return m_action_delay_enable;
}

int QTriggerDialog::actionDelayValue() const
{
	return m_action_delay_value;
}

QString QTriggerDialog::actionCommand() const
{
	return m_action_cmd;
}

int QTriggerDialog::levelACountLimit() const
{
	return m_level_a_count_limit;
}

bool QTriggerDialog::levelACheckValueEnable() const
{
	return m_level_a_check_value_enable;
}

quint8 QTriggerDialog::levelADataCheck() const
{
	return m_level_a_check_data;
}

int QTriggerDialog::levelAValueCondition() const
{
	return m_level_a_value_condition;
}

quint32 QTriggerDialog::levelAValueStart() const
{
	return m_level_a_value_start;
}

quint32 QTriggerDialog::levelAValueEnd() const
{
	return m_level_a_value_end;
}

bool QTriggerDialog::levelACheckIDEnable() const
{
	return m_level_a_check_id_enable;
}

int QTriggerDialog::levelAIDCondition() const
{
	return m_level_a_id_condition;
}

quint32 QTriggerDialog::levelAIDStart() const
{
	return m_level_a_id_start;
}

quint32 QTriggerDialog::levelAIDEnd() const
{
	return m_level_a_id_end;
}

int QTriggerDialog::levelBCountLimit() const
{
	return m_level_b_count_limit;
}

bool QTriggerDialog::levelBCheckValueEnable() const
{
	return m_level_b_check_value_enable;
}

quint8 QTriggerDialog::levelBDataCheck() const
{
	return m_level_b_check_data;
}

int QTriggerDialog::levelBValueCondition() const
{
	return m_level_b_value_condition;
}

quint32 QTriggerDialog::levelBValueStart() const
{
	return m_level_b_value_start;
}

quint32 QTriggerDialog::levelBValueEnd() const
{
	return m_level_b_value_end;
}

bool QTriggerDialog::levelBCheckIDEnable() const
{
	return m_level_b_check_id_enable;
}

int QTriggerDialog::levelBIDCondition() const
{
	return m_level_b_id_condition;
}

quint32 QTriggerDialog::levelBIDStart() const
{
	return m_level_b_id_start;
}

quint32 QTriggerDialog::levelBIDEnd() const
{
	return m_level_b_id_end;
}

void QTriggerDialog::setLevelCondition(int condition)
{
	m_level_condition = condition;
}

void QTriggerDialog::setActionDelayEnable(bool enable)
{
	m_action_delay_enable = enable;
}

void QTriggerDialog::setActionDelayValue(QString value)
{
	m_action_delay_value = value.toInt();
}

void QTriggerDialog::setActionCode(int code)
{
	ui->ediActionCommand->setVisible(code == 4);
	m_action_code = code;
}

void QTriggerDialog::setActionCommand(QString cmd)
{
	m_action_cmd = cmd;
}

void QTriggerDialog::setLevelACounterLimit(int counter)
{
	m_level_a_count_limit = counter;
}

void QTriggerDialog::setLevelACheckValueEnable(bool enable)
{
	m_level_a_check_value_enable = enable;
}

void QTriggerDialog::setLevelACheckData(bool enable, int ofs)
{
	if (enable)
		m_level_a_check_data |=  (1 << ofs);
	else
		m_level_a_check_data ^= ~(1 << ofs);
}

void QTriggerDialog::setLevelACheckData0(bool enable)
{
	setLevelACheckData(enable, 0);
}

void QTriggerDialog::setLevelACheckData1(bool enable)
{
	setLevelACheckData(enable, 1);
}

void QTriggerDialog::setLevelACheckData2(bool enable)
{
	setLevelACheckData(enable, 2);
}

void QTriggerDialog::setLevelACheckData3(bool enable)
{
	setLevelACheckData(enable, 3);
}

void QTriggerDialog::setLevelACheckData4(bool enable)
{
	setLevelACheckData(enable, 4);
}

void QTriggerDialog::setLevelACheckData5(bool enable)
{
	setLevelACheckData(enable, 5);
}

void QTriggerDialog::setLevelACheckData6(bool enable)
{
	setLevelACheckData(enable, 6);
}

void QTriggerDialog::setLevelACheckData7(bool enable)
{
	setLevelACheckData(enable, 7);
}

void QTriggerDialog::setLevelAValueCondition(int condition)
{
	ui->ediLevAValueEnd->setEnabled(condition > 3);
	m_level_a_value_condition = condition;
}

void QTriggerDialog::setLevelAValueStart(QString value)
{
	m_level_a_value_start = value.toInt(NULL, 16);
}

void QTriggerDialog::setLevelAValueEnd(QString value)
{
	m_level_a_value_end = value.toInt(NULL, 16);
}

void QTriggerDialog::setLevelACheckIdEnable(bool enable)
{
	m_level_a_check_id_enable = enable;
}

void QTriggerDialog::setLevelAIdCondition(int condition)
{
	ui->ediLevAIDEnd->setEnabled(condition > 3);
	m_level_a_id_condition = condition;
}

void QTriggerDialog::setLevelAIdStart(QString value)
{
	m_level_a_id_start = value.toInt(NULL, 16);
}

void QTriggerDialog::setLevelAIdEnd(QString value)
{
	m_level_a_id_end = value.toInt(NULL, 16);
}

void QTriggerDialog::setLevelBCounterLimit(int counter)
{
	m_level_b_count_limit = counter;
}

void QTriggerDialog::setLevelBCheckValueEnable(bool enable)
{
	m_level_b_check_value_enable = enable;
}

void QTriggerDialog::setLevelBCheckData(bool enable, int ofs)
{
	if (enable)
		m_level_b_check_data |=  (1 << ofs);
	else
		m_level_b_check_data ^= ~(1 << ofs);
}

void QTriggerDialog::setLevelBCheckData0(bool enable)
{
	setLevelBCheckData(enable, 0);
}

void QTriggerDialog::setLevelBCheckData1(bool enable)
{
	setLevelBCheckData(enable, 1);
}

void QTriggerDialog::setLevelBCheckData2(bool enable)
{
	setLevelBCheckData(enable, 2);
}

void QTriggerDialog::setLevelBCheckData3(bool enable)
{
	setLevelBCheckData(enable, 3);
}

void QTriggerDialog::setLevelBCheckData4(bool enable)
{
	setLevelBCheckData(enable, 4);
}

void QTriggerDialog::setLevelBCheckData5(bool enable)
{
	setLevelBCheckData(enable, 5);
}

void QTriggerDialog::setLevelBCheckData6(bool enable)
{
	setLevelBCheckData(enable, 6);
}

void QTriggerDialog::setLevelBCheckData7(bool enable)
{
	setLevelBCheckData(enable, 7);
}

void QTriggerDialog::setLevelBValueCondition(int condition)
{
	ui->ediLevBValueEnd->setEnabled(condition > 3);
	m_level_b_value_condition = condition;
}

void QTriggerDialog::setLevelBValueStart(QString value)
{
	m_level_b_value_start = value.toInt(NULL, 16);
}

void QTriggerDialog::setLevelBValueEnd(QString value)
{
	m_level_b_value_end = value.toInt(NULL, 16);
}

void QTriggerDialog::setLevelBCheckIdEnable(bool enable)
{
	m_level_b_check_id_enable = enable;
}

void QTriggerDialog::setLevelBIdCondition(int condition)
{
	ui->ediLevBIDEnd->setEnabled(condition > 3);
	m_level_b_id_condition = condition;
}

void QTriggerDialog::setLevelBIdStart(QString value)
{
	m_level_b_id_start = value.toInt(NULL, 16);
}

void QTriggerDialog::setLevelBIdEnd(QString value)
{
	m_level_b_id_end = value.toInt(NULL, 16);
}

void QTriggerDialog::initActionsConnections(void)
{
	connect(ui->btnOk, SIGNAL(clicked(bool)), this, SLOT(close()));
	connect(ui->comLevelCond, SIGNAL(currentIndexChanged(int)), this, SLOT(setLevelCondition(int)));
	connect(ui->chkActDelayEnable, SIGNAL(toggled(bool)), this, SLOT(setActionDelayEnable(bool)));
	connect(ui->ediActDelay, SIGNAL(textChanged(QString)), this, SLOT(setActionDelayValue(QString)));
	connect(ui->comAction, SIGNAL(currentIndexChanged(int)), this, SLOT(setActionCode(int)));
	connect(ui->ediActionCommand, SIGNAL(textChanged(QString)), this, SLOT(setActionCommand(QString)));

	connect(ui->spiLevATimes, SIGNAL(valueChanged(int)), this,SLOT(setLevelACounterLimit(int)));
	connect(ui->chkLevAValEnable, SIGNAL(toggled(bool)), this, SLOT(setLevelACheckValueEnable(bool)));
	connect(ui->chkLevAD0, SIGNAL(clicked(bool)), this, SLOT(setLevelACheckData0(bool)));
	connect(ui->chkLevAD1, SIGNAL(clicked(bool)), this, SLOT(setLevelACheckData1(bool)));
	connect(ui->chkLevAD2, SIGNAL(clicked(bool)), this, SLOT(setLevelACheckData2(bool)));
	connect(ui->chkLevAD3, SIGNAL(clicked(bool)), this, SLOT(setLevelACheckData3(bool)));
	connect(ui->chkLevAD4, SIGNAL(clicked(bool)), this, SLOT(setLevelACheckData4(bool)));
	connect(ui->chkLevAD5, SIGNAL(clicked(bool)), this, SLOT(setLevelACheckData5(bool)));
	connect(ui->chkLevAD6, SIGNAL(clicked(bool)), this, SLOT(setLevelACheckData6(bool)));
	connect(ui->chkLevAD7, SIGNAL(clicked(bool)), this, SLOT(setLevelACheckData7(bool)));
	connect(ui->comLevAValueCond, SIGNAL(currentIndexChanged(int)), this, SLOT(setLevelAValueCondition(int)));
	connect(ui->ediLevAValueStart, SIGNAL(textChanged(QString)), this, SLOT(setLevelAValueStart(QString)));
	connect(ui->ediLevAValueEnd, SIGNAL(textChanged(QString)), this, SLOT(setLevelAValueEnd(QString)));
	connect(ui->chkLevAIDEnable, SIGNAL(toggled(bool)), this, SLOT(setLevelACheckIdEnable(bool)));
	connect(ui->comLevAIDCond, SIGNAL(currentIndexChanged(int)), this, SLOT(setLevelAIdCondition(int)));
	connect(ui->ediLevAIDStart, SIGNAL(textChanged(QString)), this, SLOT(setLevelAIdStart(QString)));
	connect(ui->ediLevAIDEnd, SIGNAL(textChanged(QString)), this, SLOT(setLevelAIdEnd(QString)));

	connect(ui->spiLevBTimes, SIGNAL(valueChanged(int)), this,SLOT(setLevelBCounterLimit(int)));
	connect(ui->chkLevBValEnable, SIGNAL(toggled(bool)), this, SLOT(setLevelBCheckValueEnable(bool)));
	connect(ui->chkLevBD0, SIGNAL(clicked(bool)), this, SLOT(setLevelBCheckData0(bool)));
	connect(ui->chkLevBD1, SIGNAL(clicked(bool)), this, SLOT(setLevelBCheckData1(bool)));
	connect(ui->chkLevBD2, SIGNAL(clicked(bool)), this, SLOT(setLevelBCheckData2(bool)));
	connect(ui->chkLevBD3, SIGNAL(clicked(bool)), this, SLOT(setLevelBCheckData3(bool)));
	connect(ui->chkLevBD4, SIGNAL(clicked(bool)), this, SLOT(setLevelBCheckData4(bool)));
	connect(ui->chkLevBD5, SIGNAL(clicked(bool)), this, SLOT(setLevelBCheckData5(bool)));
	connect(ui->chkLevBD6, SIGNAL(clicked(bool)), this, SLOT(setLevelBCheckData6(bool)));
	connect(ui->chkLevBD7, SIGNAL(clicked(bool)), this, SLOT(setLevelBCheckData7(bool)));
	connect(ui->comLevBValueCond, SIGNAL(currentIndexChanged(int)), this, SLOT(setLevelBValueCondition(int)));
	connect(ui->ediLevBValueStart, SIGNAL(textChanged(QString)), this, SLOT(setLevelBValueStart(QString)));
	connect(ui->ediLevBValueEnd, SIGNAL(textChanged(QString)), this, SLOT(setLevelBValueEnd(QString)));
	connect(ui->chkLevBIDEnable, SIGNAL(toggled(bool)), this, SLOT(setLevelBCheckIdEnable(bool)));
	connect(ui->comLevBIDCond, SIGNAL(currentIndexChanged(int)), this, SLOT(setLevelBIdCondition(int)));
	connect(ui->ediLevBIDStart, SIGNAL(textChanged(QString)), this, SLOT(setLevelBIdStart(QString)));
	connect(ui->ediLevBIDEnd, SIGNAL(textChanged(QString)), this, SLOT(setLevelBIdEnd(QString)));

}
