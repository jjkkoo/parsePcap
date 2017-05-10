
#include <QtWidgets>

#include "mainwindow.h"

MainWindow::MainWindow()
{
    QWidget *widget = new QWidget;
    setCentralWidget(widget);

    tableView = new QTableView;
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    CustomTableModel *model = new CustomTableModel;
    tableView->setModel(model);


    QChart *chart = new QChart;
    chart->setAnimationOptions(QChart::AllAnimations);

    series = new QLineSeries;
    series->setName("Line 1");

    QVector<QPointF> points;
    for (int k = 0; k < 2000; k++)
        points.append(QPointF(k, k));

    series->replace(points);
    /*
    QVXYModelMapper *mapper = new QVXYModelMapper(this);
    mapper->setXColumn(0);
    mapper->setYColumn(1);
    mapper->setSeries(series);
    mapper->setModel(model);
    */
    QValueAxis *axisX = new QValueAxis;
    //axisX->setRange(0, 2000);
    axisX->setLabelFormat("%g");
    axisX->setTitleText("Reference Time");
    QValueAxis *axisY = new QValueAxis;
    //axisY->setRange(-1, 1);
    axisY->setTitleText("Audio level");
    chart->setAxisX(axisX, series);
    chart->setAxisY(axisY, series);
    chart->legend()->hide();
    chart->setTitle("Time Domain Plotting");

    chart->addSeries(series);
    chart->createDefaultAxes();
    chartView = new QChartView(chart);

    QSplitter *splitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(tableView);
    splitter->addWidget(chartView);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(splitter);
    widget->setLayout(layout);

    m_pProgressBar = new QProgressBar;
    statusBar()->addWidget(m_pProgressBar);

    createActions();
    createMenus();
    resize(798, 507);

    initUI();
}
/*
#ifndef QT_NO_CONTEXTMENU
void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(markAct);
    menu.exec(event->globalPos());
}
#endif // QT_NO_CONTEXTMENU
*/
void MainWindow::initUI()
{
    m_pProgressBar->hide();
    statusBar()->showMessage("");
    setWindowTitle(tr("parsePcap"));
}

void MainWindow::refreshProgress(int value)
{
    qDebug() << "Handle Thread : " << QThread::currentThreadId();
    m_pProgressBar->setValue(value);
}

void MainWindow::parseFinished()
{
    m_pProgressBar->hide();
    parseThread->deleteLater();
}

void MainWindow::startParsing(const QStringList &pathList)
{
    m_pProgressBar->show();
    parseThread = new ParseThread(this);
    connect(parseThread, &ParseThread::resultReady, this, &MainWindow::refreshProgress);
    connect(parseThread, &ParseThread::finished, this, &MainWindow::parseFinished);
    parseThread->start();
}

void MainWindow::pickFile()
{
    qDebug() << "pickFile action";

    QStringList pathList = QFileDialog::getOpenFileNames(this, tr("Pick Pcap Files"),  ".",
                                                tr("Pcap Files(*.pcap);;All Files(*.*)"));
    qDebug() << pathList;
    if(!pathList.isEmpty()) {
        foreach (const QString &str, pathList)
            qDebug() << str;
        startParsing(pathList);

    } else {
        statusBar()->showMessage("Pick Pcap Files Action Cancelled");
    }
}

void MainWindow::clearFile()
{
    qDebug() << "clearFile action";
}

void MainWindow::exit()
{
    qDebug() << "exit action";
}

void MainWindow::plot()
{
    qDebug() << "plot action";
}

void MainWindow::exportMedia()
{
    qDebug() << "exportMedia action";
}

void MainWindow::play()
{
    qDebug() << "play action";
}

void MainWindow::stop()
{
    qDebug() << "stop action";
}
/*
void MainWindow::mark()
{
    //editor1->setText(tr("Invoked <b>Edit|Cut</b>"));
}
*/

void MainWindow::createActions()
{
    pickAct = new QAction(tr("&Pick a Pcap File"), this);
    pickAct->setShortcut(tr("F4"));
    pickAct->setStatusTip(tr("Pick a Pcap File"));
    connect(pickAct, &QAction::triggered, this, &MainWindow::pickFile);

    clearAct = new QAction(tr("&Clear"), this);
    clearAct->setShortcut(tr("F8"));
    clearAct->setStatusTip(tr("Clear"));
    connect(clearAct, &QAction::triggered, this, &MainWindow::clearFile);

    exitAct = new QAction(tr("&Exit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit"));
    connect(exitAct, &QAction::triggered, this, &MainWindow::exit);

    plotAct = new QAction(tr("&Plot"), this);
    plotAct->setShortcut(tr("F5"));
    plotAct->setStatusTip(tr("Plot"));
    connect(plotAct, &QAction::triggered, this, &MainWindow::plot);

    exportAct = new QAction(tr("&Export"), this);
    exportAct->setShortcut(tr("F6"));
    exportAct->setStatusTip(tr("Export"));
    connect(exportAct, &QAction::triggered, this, &MainWindow::exportMedia);

    playAct = new QAction(tr("&Play/Pause/Resume"), this);
    playAct->setShortcut(tr("Space"));
    playAct->setStatusTip(tr("Play/Pause/Resume"));
    connect(playAct, &QAction::triggered, this, &MainWindow::play);

    stopAct = new QAction(tr("&Play/Pause/Resume"), this);
    stopAct->setShortcut(tr("F9"));
    stopAct->setStatusTip(tr("Stop"));
    connect(stopAct, &QAction::triggered, this, &MainWindow::stop);
/*
    markAct = new QAction(tr("&Mark"), this);
    markAct->setShortcut(tr("F2"));
    markAct->setStatusTip(tr("Mark"));
    connect(markAct, &QAction::triggered, this, &MainWindow::mark);
*/
//    leftAlignAct->setChecked(true);
}

void MainWindow::createMenus()
{

    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(pickAct);
    fileMenu->addAction(clearAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(exportAct);
    editMenu->addAction(plotAct);

    playMenu = editMenu->addMenu(tr("&Play"));
    playMenu->addAction(playAct);
    playMenu->addAction(stopAct);

    modesMenu= menuBar()->addMenu(tr("&Modes"));

    helpMenu = menuBar()->addMenu(tr("&Help"));
//    helpMenu->addAction(aboutAct);
//    helpMenu->addAction(aboutQtAct);

}

