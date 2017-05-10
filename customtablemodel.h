#ifndef CUSTOMTABLEMODEL_H
#define CUSTOMTABLEMODEL_H

#include <QtCore/QAbstractTableModel>
#include <QtCore/QHash>
#include <QtCore/QRect>

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

private:
    QList<QVector<QString> * > m_data;
    int m_columnCount;
    int m_rowCount;
    const QStringList columnHeader{"source ip","srcPort","dest ip","destPort","first packet time","last packet time","pktCount","PT","SSRC","codec","Lost","Dup","WrongSeq","MaxDelta(s/seq)"};

    //QList<QStringList> tableData;
};

#endif // CUSTOMTABLEMODEL_H
