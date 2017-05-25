
#include "CustomTableModel.h"

#include <QDebug>

CustomTableModel::CustomTableModel(QObject *parent) : m_rowCount(0), m_data(QList<QStringList>()),
    QAbstractTableModel(parent)
{
    //qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));

    //m_columnCount = 12;
    //m_rowCount = 0;

    // m_data
//    for (int i = 0; i < m_rowCount; i++) {
//        QVector<QString>* dataVec = new QVector<QString>(columnHeader.size());
//        for (int k = 0; k < dataVec->size(); k++) {
//            if (k % 2 == 0)
//                dataVec->replace(k, QString(i * 50 + qrand() % 20));
//            else
//                dataVec->replace(k, QString(qrand() % 100));
//        }
//        m_data.append(dataVec);
//    }
}

int CustomTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_data.count();
}

int CustomTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return columnHeader.size();
}

QVariant CustomTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        return columnHeader.at(section);

    } else {
        return QString("%1").arg(section + 1);
    }
}

QVariant CustomTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        return m_data[index.row()].at(index.column());
    } else if (role == Qt::EditRole) {
        return m_data[index.row()].at(index.column());
    } else if (role == Qt::BackgroundRole) {
        if (index.row() % 2 == 1)
            return QColor("#E6E6FA");    //lavender http://www.wahart.com.hk/rgb.htm
        else
            return QColor(Qt::white);
    }
    return QVariant();
}

bool CustomTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        m_data[index.row()].replace(index.column(), value.toString());
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags CustomTableModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}
/*
bool CustomTableModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    beginInsertRows(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row) {
        m_data.insert(position, "");
    }

    endInsertRows();
    return true;
}

bool CustomTableModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row) {
        m_data.removeAt(position);
    }

    endRemoveRows();
    return true;
}
*/

bool CustomTableModel::appendData(QList<QStringList> data)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount()+data.size()-1);
    m_data.append(data);
    endInsertRows();

    return true;
}

bool CustomTableModel::clearData()
{
    if( m_data.count() ) {
        beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
        m_data.clear();
        endRemoveRows();
    }
    return true;
}


QStringList CustomTableModel::getLine(int index) {
    return m_data.at(index).mid(0,4);
}
