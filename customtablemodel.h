#ifndef CUSTOMTABLEMODEL_H
#define CUSTOMTABLEMODEL_H

#include <QtCore/QAbstractTableModel>
#include <QtCore/QHash>
#include <QtCore/QRect>
#include <QColor>

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

class CustomTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit CustomTableModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;
//    bool insertRows(int position, int rows, const QModelIndex &parent);
//    bool removeRows(int position, int rows, const QModelIndex &parent);
    bool appendData(QList<QStringList> data);
    bool clearData();
    QStringList getLine(int index);

private:
    int m_columnCount;
    int m_rowCount;

    const QStringList columnHeader{"source ip","srcPort","dest ip","destPort","first packet time","last packet time","pktCount","PT","SSRC","codec","Lost","Dup","WrongSeq","MaxDelta(s/seq)"};
    QList<QStringList> m_data;

};

#endif // CUSTOMTABLEMODEL_H
