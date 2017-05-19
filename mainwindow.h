
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


//#include <QtCharts/QChart>
//#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
//#include <QtCharts/QVXYModelMapper>
#include <QtWidgets/QHeaderView>

#include "customchart.h"
#include "chartview.h"
#include "customtablemodel.h"
#include "parsethread.h"
#include "decodethread.h"
#include "comboboxdelegate.h"

//QT_CHARTS_USE_NAMESPACE


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

protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void pickFile();
    void clearTable();
    void exit();
    void plot();
    void exportMedia();
    void play();
    void stop();
    //void mark();
    void refreshProgress(int value);
    void parseFinished(QList<QStringList> parsedList, QList<QTemporaryFile *> fileNameList);//QStringList fileNameList);
    void decodeFinished(QTemporaryFile * decodeResult);
    void lastWords(const QString & laswords);
    void parseThrOver();
    void decodeThrOver();
    void plotOnChart(QList<int>indexList);
    void cancelAll();
    void copySelection();


private:
    void createActions();
    void createMenus();
    void initUI();
    void readSettings();
    void writeSettings();
    void startParsing(const QString filePath);
    void startDecoding(int index);

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
    CustomTableModel *tableModel;
    ChartView *chartView;
    Chart *chart;
    QLineSeries *series;
    QProgressBar *m_pProgressBar;
    ParseThread *parseThread;
    DecodeThread *decodeThread;
    //QList <ParseThread> *parseThreadList;

    QStringList *parseJobList;
    QString pickDir;
    QString saveDir;
    QList<QTemporaryFile *> m_tempMediaFile;
    QVector<QTemporaryFile *> m_tempDecodedFile;

    QList<int> waitForPlotList;
};

#endif
