#include "dialogmodel.h"
#include <QDebug>

dialogModel::dialogModel(QObject *parent) :
    QAbstractTableModel(parent)
{

}

int dialogModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_rowCount;
}

int dialogModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_columnCount;
}

QVariant dialogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        return columnHeader.at(section);
        //qDebug() << columnHeader.at(section);
    }
    return QVariant();
}

QVariant dialogModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        //qDebug() << m_data[index.row()];
        if (index.column() == 1)
            return m_data[index.row()];
        else if (index.column() == 0)
            return index.row() + 96;
    } else if (role == Qt::EditRole) {
        if (index.column() == 1)
            return m_data[index.row()];
    } /*else if (role == Qt::BackgroundRole) {
        foreach (QRect rect, m_mapping) {
            if (rect.contains(index.column(), index.row()))
                return QColor(m_mapping.key(rect));
        }
        // cell not mapped return white color
        return QColor(Qt::white);
    }*/
    return QVariant();
}

bool dialogModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        m_data[index.row()] = value.toInt();
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags dialogModel::flags(const QModelIndex &index) const
{
    if (index.column() == 1)
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    return QAbstractItemModel::flags(index);
}
/*
bool dialogModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    beginInsertRows(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row) {
        m_data.insert(position, "");
    }

    endInsertRows();
    return true;
}

bool dialogModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row) {
        m_data.removeAt(position);
    }

    endRemoveRows();
    return true;
}
*/

