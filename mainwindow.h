
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
//class QAudioDeviceInfo;
//class QChart;
//class QChartView;
//class QHeaderView;
//class QLineSeries;
QT_END_NAMESPACE

//using QAudio::State;

#include <QtCharts/QChart>
//#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
//#include <QtCharts/QVXYModelMapper>
#include <QtWidgets/QHeaderView>
#include <QAudio>
#include <QAudioFormat>
#include <QAudioOutput>

//#include "customchart.h"
#include "chartview.h"
#include "customtablemodel.h"
#include "parsethread.h"
#include "decodethread.h"
#include "comboboxdelegate.h"
#include "tabdialog.h"
#include "wavefile.h"
#include "customtableview.h"

//QT_CHARTS_USE_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void pickFile();
    void clearTable();
    void exit();
    void plot();
    void exportMedia();
    void play();
    void stop();
    void showPreference();
    void refreshProgress(int value);
    void parseFinished(QList<QStringList> parsedList, QList<QTemporaryFile *> fileNameList);//QStringList fileNameList);
    void decodeFinished(QTemporaryFile * decodeResult);
    void lastWords(const QString & laswords);
    void parseThrOver();
    void decodeThrOver();
    void plotOnChart(QList<int>indexList);
    void cancelAll();
    void copySelection();
//    void playerRefreshProgress();
    void handleStateChanged(QAudio::State newState);
    void pushTimerExpired();
    void updatePlayfilePos(double posPercent);
    void toggleMark();
    void seekMark();
    void selectAll();
    void showAllOneSecondPoints();
    void showAllSampledPoints();

private:
    void createActions();
    void createMenus();
    void initUI();
    void setupPosition();
    void savePosition();
    void startParsing(const QString filePath);
    void startDecoding(int index);
    void exportAfterDecode(QList<int> indexList);
    void saveMediaFile(int index, QString directory);
    void removeTempFile();

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *playMenu;
    QMenu *modesMenu;
    QMenu *settingsMenu;
    QMenu *helpMenu;

    QAction *pickAct;
    QAction *clearAct;
    QAction *exitAct;
    QAction *plotAct;
    QAction *exportAct;
    QAction *playAct;
    QAction *stopAct;
    QAction *preferenceAct;
    QAction *toggleMarkAct;
    QAction *seekMarkAct;

    QTableView *tableView;
    CustomTableModel *tableModel;
    ChartView *chartView;
    QSplitter *splitter;
    QChart *chart;
    QLineSeries *series;
    QProgressBar *m_pProgressBar;
    ParseThread *parseThread;
    DecodeThread *decodeThread;
    //QList <ParseThread> *parseThreadList;

    QStringList *parseJobList;
    QList<QTemporaryFile *> m_tempMediaFile;
    QVector<QTemporaryFile *> m_tempDecodedFile;
    QVector<int> m_codecVector;

    QList<int> waitForPlotList;
    QList<int> waitForExportList;

    int currentPlotIndex;
    QTemporaryFile *PlayerFile;
    QAudioOutput *audio;
    int currentSampleRate;
    unsigned int currentFileSize;
    unsigned int currentPlayPos;
    unsigned int startPlayPos;

    QIODevice *m_output;
    QTimer *m_pushTimer;
    char m_buffer[32768];//16000/50 * 2];
};

#endif
