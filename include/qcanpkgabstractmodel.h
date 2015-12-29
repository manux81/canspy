#ifndef QCANPKGABSTRACTMODEL_H
#define QCANPKGABSTRACTMODEL_H

#include "canbus/can_packet.h"


#include <QAbstractTableModel>
#include <QModelIndex>
#include <QMimeData>
#include <QString>
#include <QTime>

class QCanPkgAbstractModel : public QAbstractTableModel
{
	Q_OBJECT

	public:
		typedef struct{
			QString id;
			QString flags;
			QString length;
			QString timestamp;
			QString elapsed;
			QString data;
			QString direction;
		} can_str_packet_t;

		typedef QList<can_str_packet_t> CanPkgBuffer;

		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		int columnCount(const QModelIndex &parent = QModelIndex()) const;
		QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
		bool setData(const QModelIndex &index, const QVariant &value, int role);
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

		Qt::ItemFlags flags(const QModelIndex &index) const;
		QStringList mimeTypes() const;
		QMimeData* mimeData(const QModelIndexList &indexes) const;

		virtual bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex());
		virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

		bool removeDisContinousRows(int position, int rows);
		bool removeContinousRows(int position, int rows);

		CanPkgBuffer msgList;

	public slots:
		virtual void messageEnqueued(can_packet_t canpkg) = 0;

	public:
		QCanPkgAbstractModel(QObject *parent = 0);
		~QCanPkgAbstractModel();
	protected:
		QTime m_startTime;
};


#endif
