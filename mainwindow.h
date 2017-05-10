
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QSplitter;
class QTableView;
class QXYSeries;
class QProgressBar;
class QDialog;
class QStringList;
//class QChart;
//class QChartView;
//class QHeaderView;
//class QLineSeries;
QT_END_NAMESPACE


#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QVXYModelMapper>
#include <QtWidgets/QHeaderView>
#include <QtCharts/QValueAxis>
#include "customchartmodel.h"
#include "customtablemodel.h"
#include "parsethread.h"

QT_CHARTS_USE_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
/*
protected:
#ifndef QT_NO_CONTEXTMENU
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
#endif // QT_NO_CONTEXTMENU
*/
private slots:
    void pickFile();
    void clearFile();
    void exit();
    void plot();
    void exportMedia();
    void play();
    void stop();
    //void mark();
    void refreshProgress(int value);
    void parseFinished();


private:
    void createActions();
    void createMenus();
    void initUI();
    void startParsing(const QStringList &pathList);

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *playMenu;
    QMenu *modesMenu;
    QMenu *helpMenu;
    QAction *pickAct;
    QAction *clearAct;
    QAction *exitAct;
    QAction *plotAct;
    QAction *exportAct;
    QAction *playAct;
    QAction *stopAct;
    //QAction *markAct;
    QTableView *tableView;
    QChartView *chartView;
    QLineSeries *series;
    QProgressBar *m_pProgressBar;
    QList <ParseThread> *parseThreadList;
    QStringList *parseJobList;
};

#endif
