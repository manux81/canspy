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


#ifndef TRIGGERDIALOG_H
#define TRIGGERDIALOG_H

#include "ui_triggerdialog.h"

#include <QObject>
#include <QWidget>
#include <QDialog>

namespace Ui {
class QTriggerDialog;
}

class QTriggerDialog : public QDialog
{
	Q_OBJECT
	Q_PROPERTY(int     levelCondition         READ levelCondition)
	Q_PROPERTY(int     action                 READ action)
	Q_PROPERTY(bool    actionDelay            READ actionDelay)
	Q_PROPERTY(int     actionDelayValue       READ actionDelayValue)
	Q_PROPERTY(QString actionCommand          READ actionCommand)
	Q_PROPERTY(int     levelACountLimit       READ levelACountLimit)
	Q_PROPERTY(bool    levelACheckValueEnable READ levelACheckValueEnable)
	Q_PROPERTY(quint8  levelADataCheck        READ levelADataCheck)
	Q_PROPERTY(int     levelAValueCondition   READ levelAValueCondition)
	Q_PROPERTY(quint32 levelAValueStart       READ levelAValueStart)
	Q_PROPERTY(quint32 levelAValueEnd         READ levelAValueEnd)
	Q_PROPERTY(bool    levelACheckIDEnable    READ levelACheckIDEnable)
	Q_PROPERTY(int     levelAIDCondition      READ levelAIDCondition)
	Q_PROPERTY(quint32 levelAIDStart          READ levelAIDStart)
	Q_PROPERTY(quint32 levelAIDEnd            READ levelAIDEnd)
	Q_PROPERTY(int     levelBCountLimit       READ levelBCountLimit)
	Q_PROPERTY(bool    levelBCheckValueEnable READ levelBCheckValueEnable)
	Q_PROPERTY(quint8  levelBDataCheck        READ levelBDataCheck)
	Q_PROPERTY(int     levelBValueCondition   READ levelBValueCondition)
	Q_PROPERTY(quint32 levelBValueStart       READ levelBValueStart)
	Q_PROPERTY(quint32 levelBValueEnd         READ levelBValueEnd)
	Q_PROPERTY(bool    levelBCheckIDEnable    READ levelBCheckIDEnable)
	Q_PROPERTY(int     levelBIDCondition      READ levelBIDCondition)
	Q_PROPERTY(quint32 levelBIDStart          READ levelBIDStart)
	Q_PROPERTY(quint32 levelBIDEnd            READ levelBIDEnd)



public:
	explicit QTriggerDialog(QWidget *parent= 0);
	~QTriggerDialog() {}
	int levelCondition(void) const;
	int action() const;
	bool actionDelay(void) const;
	int actionDelayValue(void) const;
	QString actionCommand(void) const;
	int levelACountLimit(void) const;
	bool levelACheckValueEnable(void) const;
	quint8 levelADataCheck(void) const;
	int levelAValueCondition(void) const;
	quint32 levelAValueStart(void) const;
	quint32 levelAValueEnd(void) const;
	bool levelACheckIDEnable(void) const;
	int levelAIDCondition(void) const;
	quint32 levelAIDStart(void) const;
	quint32 levelAIDEnd(void) const;
	int levelBCountLimit(void) const;
	bool levelBCheckValueEnable(void) const;
	quint8 levelBDataCheck(void) const;
	int levelBValueCondition(void) const;
	quint32 levelBValueStart(void) const;
	quint32 levelBValueEnd(void) const;
	bool levelBCheckIDEnable(void) const;
	int levelBIDCondition(void) const;
	quint32 levelBIDStart(void) const;
	quint32 levelBIDEnd(void) const;



private slots:
	void setLevelCondition(int);
	void setActionDelayEnable(bool);
	void setActionDelayValue(QString);
	void setActionCode(int);
	void setActionCommand(QString);

	/* Level A */
	void setLevelACounterLimit(int);
	void setLevelACheckValueEnable(bool);
	void setLevelACheckData0(bool);
	void setLevelACheckData1(bool);
	void setLevelACheckData2(bool);
	void setLevelACheckData3(bool);
	void setLevelACheckData4(bool);
	void setLevelACheckData5(bool);
	void setLevelACheckData6(bool);
	void setLevelACheckData7(bool);
	void setLevelAValueCondition(int);
	void setLevelAValueStart(QString);
	void setLevelAValueEnd(QString);
	void setLevelACheckIdEnable(bool);
	void setLevelAIdCondition(int);
	void setLevelAIdStart(QString);
	void setLevelAIdEnd(QString);
	/* Level B */
	void setLevelBCounterLimit(int);
	void setLevelBCheckValueEnable(bool);
	void setLevelBCheckData0(bool);
	void setLevelBCheckData1(bool);
	void setLevelBCheckData2(bool);
	void setLevelBCheckData3(bool);
	void setLevelBCheckData4(bool);
	void setLevelBCheckData5(bool);
	void setLevelBCheckData6(bool);
	void setLevelBCheckData7(bool);
	void setLevelBValueCondition(int);
	void setLevelBValueStart(QString);
	void setLevelBValueEnd(QString);
	void setLevelBCheckIdEnable(bool);
	void setLevelBIdCondition(int);
	void setLevelBIdStart(QString);
	void setLevelBIdEnd(QString);

private:
	void initActionsConnections(void);
	void setLevelACheckData(bool, int);
	void setLevelBCheckData(bool, int);

	Ui::triggerdialog *ui;

	int m_level_condition;
	bool m_action_delay_enable;
	quint32 m_action_delay_value;
	quint8 m_action_code;

	int m_level_a_count_limit;
	bool m_level_a_check_value_enable;
	quint8 m_level_a_check_data;
	int m_level_a_value_condition;
	quint32	m_level_a_value_start;
	quint32	m_level_a_value_end;
	bool m_level_a_check_id_enable;
	int m_level_a_id_condition;
	quint32 m_level_a_id_start;
	quint32 m_level_a_id_end;


	int m_level_b_count_limit;
	bool m_level_b_check_value_enable;
	quint8 m_level_b_check_data;
	int m_level_b_value_condition;
	quint32	m_level_b_value_start;
	quint32	m_level_b_value_end;
	bool m_level_b_check_id_enable;
	int m_level_b_id_condition;
	quint32 m_level_b_id_start;
	quint32 m_level_b_id_end;
	QString m_action_cmd;

};

#endif
