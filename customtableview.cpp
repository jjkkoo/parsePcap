#include "customtableview.h"

CustomTableView::CustomTableView(QWidget *parent) : QTableView(parent), markedList(std::set<int>())
{

}


#ifndef QT_NO_CONTEXTMENU
    void CustomTableView::contextMenuEvent(QContextMenuEvent *event) {
        qDebug() << "contextMenuEvent";
    }

#endif // QT_NO_CONTEXTMENU

void CustomTableView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_F2:
        if (event->modifiers() == Qt::ControlModifier) {
            QItemSelectionModel * selection = selectionModel();
            QModelIndexList indexes = selection->selectedIndexes();
            if (indexes.size() <= 0)    return;
            QSet<int> indexRowSet;
            foreach(const QModelIndex& index, indexes){
                indexRowSet.insert(index.row());
            }
            QList<int> indexList = indexRowSet.toList();

            emit markIt(indexList);

            foreach(const int& index, indexList) {
                std::set<int>::iterator it = markedList.find(index);
                if (it == markedList.end()){
                    markedList.insert(index);
                }
                else {
                    markedList.erase(it);
                }
            }
            break;
        }

        for( auto target = markedList.begin(); target != markedList.end(); ++target) {
            qDebug() << *target;
        }

        break;

    default:
        break;
    }
}
