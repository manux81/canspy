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


#include <QAbstractTableModel>
#include <QVariant>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QDateTime>
#include <QByteArray>
#include <QStringList>
#include <QTextStream>
#include <QDebug>

#include "qcanpkgabstractmodel.h"
#include <string.h>

QCanPkgAbstractModel::QCanPkgAbstractModel(QObject *parent)
	: QAbstractTableModel(parent)
{
}

QCanPkgAbstractModel::~QCanPkgAbstractModel()
{
}

int QCanPkgAbstractModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return msgList.count();
}

int QCanPkgAbstractModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return 7;
}

QVariant QCanPkgAbstractModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	QVariant ret = QAbstractTableModel::headerData(section, orientation, role);
	if(orientation == Qt::Horizontal) {
		if(role == Qt::DisplayRole) {
			switch(section) {
			case 0 :
				return tr("ID");
				break;
			case 1:
				return tr("Flags");
				break;
			case 2 :
				return tr("TimeStamp");
				break;
			case 3 :
				return tr("Elapsed");
				break;
			case 4 :
				return tr("DLC");
				break;
			case 5 :
				return tr("Data");
				break;
			case 6 :
				return tr("Dir");
				break;
			default :
				break;
			}
		}
	}
	return ret;
}

QVariant QCanPkgAbstractModel::data(const QModelIndex &index, int role) const
{
	QVariant ret;

	if(!index.isValid()) {
		qDebug()<<"Invalid Index of data provided!";
		return ret;
	}
	int row = index.row();
	int col = index.column();

	if(row >= 0 ) {
		if (role == Qt::TextAlignmentRole ) {
			switch (col) {
			case 3:
				return QVariant(Qt::AlignRight | Qt::AlignVCenter);
			case 5:
				return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
			default:
				return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
			}
		}
		if(role == Qt::DisplayRole) {
			const can_str_packet_t &canpkg = msgList.at(row);
			switch(col) {
			case 0 : {
				ret = QVariant(canpkg.id);

			}
			break;
			case 1: {
				ret = QVariant(canpkg.flags);
			}
			break;
			case 2 : {
				ret = QVariant(canpkg.timestamp);
			}
			break;
			case 3 : {
				ret = QVariant(canpkg.elapsed);
			}
			break;
			case 4 : {
				ret = QVariant(canpkg.length);
			}
			break;
			case 5 : {
				ret = QVariant(canpkg.data);
			}
			break;
			case 6 : {
				ret = QVariant(canpkg.direction);
			}
			default :
				break;
			}
		}
	}

	return ret;
}

bool QCanPkgAbstractModel::setData(const QModelIndex &index, const QVariant &value, int rule)
{
	return QAbstractTableModel::setData(index, value, rule);
}

Qt::ItemFlags QCanPkgAbstractModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

	if (index.isValid()) {
		return Qt::ItemIsDragEnabled | defaultFlags;
	} else
		return defaultFlags;
}

QMimeData* QCanPkgAbstractModel::mimeData(const QModelIndexList &indexes) const
{
	QMimeData *mimeData = new QMimeData();
	QByteArray encodedData;

	QTextStream stream(&encodedData, QIODevice::WriteOnly);

	int oldRow=-1;
	foreach (QModelIndex index, indexes) {
		int actualRow = index.row();
		if(oldRow == actualRow) {
			continue;
		} else {
			for(int i = 0; i < columnCount(); i++) {
				if((i==2) || (i==3)) {
					continue;
				} else {
					QModelIndex index2 = this->index(actualRow, i);
					QString text = this->data(index2, Qt::DisplayRole).toString();
					stream << text <<"\t";
				}
			}
			stream << endl;
		}
		oldRow = actualRow;
	}

	mimeData->setData("application/message", encodedData);
	return mimeData;
}

QStringList QCanPkgAbstractModel::mimeTypes() const
{
	QStringList types;
	types << "application/message";
	return types;
}

bool QCanPkgAbstractModel::insertRows(int position, int rows, const QModelIndex &parent)
{
	Q_UNUSED(parent);
	beginInsertRows(QModelIndex(), position, position+rows-1);
	for (int row = 0; row < rows; ++row) {
		can_str_packet_t packet;
		msgList.insert(position, packet);
	}
	endInsertRows();

	return true;

}

bool QCanPkgAbstractModel::removeRows(int row, int count, const QModelIndex &parent)
{
	Q_UNUSED(parent);

	if (count == 0)
		return true;

	beginRemoveRows(QModelIndex(), row, row+count-1);
	for (int r = row; r < count; ++r) {
		msgList.removeAt(0);
	}
	endRemoveRows();

	return true;
}

bool QCanPkgAbstractModel::removeDisContinousRows(int position, int rows)
{
	beginRemoveRows(QModelIndex(), position, position+rows-1);
	msgList.removeAt(position);
	endRemoveRows();

	return true;
}

bool QCanPkgAbstractModel::removeContinousRows(int position, int rows)
{
	beginRemoveRows(QModelIndex(), position, position+rows-1);
	for (int row = 0; row < rows; ++row) {
		msgList.removeAt(position);
	}
	endRemoveRows();

	return true;
}

