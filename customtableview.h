#ifndef CUSTOMTABLEVIEW_H
#define CUSTOMTABLEVIEW_H

#include<set>

#include <QDebug>
#include <QTableView>
#include <QKeyEvent>
//#include <QList>

class CustomTableView : public QTableView
{
    Q_OBJECT

public:
    CustomTableView(QWidget *parent = 0);

signals:
    void markIt(QList<int> targetList);

protected:
#ifndef QT_NO_CONTEXTMENU
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
#endif // QT_NO_CONTEXTMENU

    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

private:
    //QSet<int> markedList;
    std::set<int> markedList;
};

#endif // CUSTOMTABLEVIEW_H
