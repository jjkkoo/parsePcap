#ifndef COMBOBOXDELEGATE_H
#define COMBOBOXDELEGATE_H

#include <QStringList>
#include <QItemDelegate>

#ifndef CODEC_H
#define CODEC_H
enum codecColumns
{
    COL_amr,
    COL_amr_wb,
    COL_amr_octet_align,
    COL_amr_wb_octet_align,
    COL_h264,
    COL_DTMF,
    COL_EVS
};
#endif

class QModelIndex;
class QWidget;
class QVariant;

class ComboBoxDelegate : public QItemDelegate
{
Q_OBJECT
public:
    ComboBoxDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    //void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    const QStringList items {"amr", "amr-wb", "amr_octet-align", "amr-wb_octet-align", "h264", "DTMF", "EVS"};

};
#endif
