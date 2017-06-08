#ifndef DIALOGMODEL_H
#define DIALOGMODEL_H

#include <QtCore/QAbstractTableModel>
#include <QtCore/QHash>
#include <QtCore/QRect>
#include <QVector>

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
