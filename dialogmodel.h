#ifndef DIALOGMODEL_H
#define DIALOGMODEL_H

#include <QtCore/QAbstractTableModel>
#include <QtCore/QHash>
#include <QtCore/QRect>
#include <QVector>

#ifndef COLUMNS_H
#define COLUMNS_H
enum tableColumns
{
    COL_source_ip,
    COL_srcPort,
    COL_dest_ip,
    COL_destPort,
    COL_first_packet_time,
    COL_last_packet_time,
    COL_pktCount,
    COL_PT,
    COL_SSRC,
    COL_codec,
    COL_Lost,
    COL_Dup,
    COL_WrongSeq,
    COL_MaxDelta
};
#endif

class dialogModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit dialogModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;
//    bool insertRows(int position, int rows, const QModelIndex &parent);
//    bool removeRows(int position, int rows, const QModelIndex &parent);


private:
    const int m_columnCount = 2;
    const int m_rowCount = 32;
    const QVector<QString> columnHeader{"Payload Type", "Codec"};

    //QVector<int> m_data;
    int m_data[32];
};

#endif // DIALOGMODEL_H
