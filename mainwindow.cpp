
#include <QtWidgets>

#include "mainwindow.h"

MainWindow::MainWindow()
{
    QWidget *widget = new QWidget;
    setCentralWidget(widget);

    tableView = new QTableView;
    //tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->resizeColumnsToContents();

    tableModel = new CustomTableModel;
    tableView->setModel(tableModel);

    ComboBoxDelegate* delegate = new ComboBoxDelegate;
    tableView->setItemDelegateForColumn(COL_codec, delegate);

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
    //resize(798, 507);

    parseThread = nullptr;

    initUI();
    readSettings();
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
    chartView->hide();
    statusBar()->showMessage("");
    setWindowTitle(tr("parsePcap"));
}

void MainWindow::writeSettings()
{
    QSettings settings("jjkkoo", "parsePcap");

    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.setValue("pickDirectory", pickDir);
    settings.setValue("saveDirectory", saveDir);
    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings("jjkkoo", "parsePcap");

    settings.beginGroup("MainWindow");
    resize(settings.value("size", QSize(798, 507)).toSize());
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    pickDir = settings.value("pickDirectory", QCoreApplication::applicationDirPath()).toString();
    saveDir = settings.value("saveDirectory", QCoreApplication::applicationDirPath()).toString();
    settings.endGroup();
    qDebug() << QCoreApplication::applicationDirPath()<< "," << QDir::currentPath() << "," << QDir::current().path();
    qDebug() << pickDir;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (parseThread == nullptr) {
        qDebug() << "ready to exit";
        event->accept();
        //return;
    }
    else {
        const QMessageBox::StandardButton ret
            = QMessageBox::warning(this, tr("ParsePcap Exiting Check"),
                                   tr("Unfinished Actions Detected\n"
                                      "Really Exiting?"),
                                   QMessageBox::Yes | QMessageBox::Cancel);
        switch (ret) {
            case QMessageBox::Yes:
                if (parseThread != nullptr and parseThread->isRunning()) {
                    parseThread->stopMe();
                }
                writeSettings();
                event->accept();
                break;
            case QMessageBox::Cancel:
            default:
                event->ignore();
                break;
        }
    }
}

void MainWindow::refreshProgress(int value)
{
    m_pProgressBar->setValue(value);
}

void MainWindow::parseFinished(QList<QStringList> parsedList)
{
    int beforeAppendRowCount = tableModel->rowCount();
    tableModel->appendData(parsedList);

    for ( int i = beforeAppendRowCount; i < tableModel->rowCount(); ++i ) {
        tableView->openPersistentEditor( tableModel->index(i, COL_codec) );
    }
    tableView->resizeColumnsToContents();
}

void MainWindow::parseThrOver()
{
    m_pProgressBar->hide();
    parseThread->deleteLater();
    parseThread = nullptr;
}

void MainWindow::cancelAll()
{
    if (parseThread->isRunning()){
        parseThread->stopMe();
    }
}

void MainWindow::copySelection()
{
    QItemSelectionModel * selection = tableView->selectionModel();
    QModelIndexList indexes = selection->selectedIndexes(); //tableView->selectedIndexes()
    QStringList list ;
    QModelIndex previous = indexes.first();
    foreach ( const QModelIndex& index, indexes )
    {
        if (index.row() != previous.row())
        {
            list.append("\n");
            previous = index;
        }
       list << index.data().toString() ;
    }

    QApplication::clipboard()->setText( list.join( ", " ) ) ;
}

void MainWindow::startParsing(const QString filePath)
{
    m_pProgressBar->show();
    parseThread = new ParseThread(filePath, this);
    connect(parseThread, &ParseThread::updateProgress, this, &MainWindow::refreshProgress);
    connect(parseThread, &ParseThread::parseSuccess, this, &MainWindow::parseFinished);
    connect(parseThread, &ParseThread::finished, this, &MainWindow::parseThrOver);
    parseThread->start();
}

void MainWindow::pickFile()
{
    qDebug() << "pickFile action";

    QStringList pathList = QFileDialog::getOpenFileNames(this, tr("Pick Pcap Files"),  pickDir,
                                                tr("Pcap Files(*.pcap);;All Files(*.*)"));
    qDebug() << pathList;
    if(!pathList.isEmpty()) {
        pickDir = QFileInfo(pathList[0]).absolutePath() ;
        foreach (const QString &str, pathList)
            startParsing(str);

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
    close();
}

void MainWindow::plot()
{
    qDebug() << "plot action";
    QItemSelectionModel * selection = tableView->selectionModel();
    QModelIndexList indexes = selection->selectedIndexes();
    QSet<int> indexSet;
    foreach(const QModelIndex& index, indexes){
        indexSet.insert(index.row());
    }
    QList<int> indexList = indexSet.toList();
    qSort(indexList.begin(), indexList.end());
    qDebug() << indexList;
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

    QShortcut * escShortcut = new QShortcut(QKeySequence::Cancel, this);
    connect(escShortcut, &QShortcut::activated, this, &MainWindow::cancelAll);
    //connect(escShortcut, SIGNAL(activated()), this, SLOT(cancelAll()));

    QShortcut * copyShortcut = new QShortcut(QKeySequence::Copy, this);
    connect(copyShortcut, &QShortcut::activated, this, &MainWindow::copySelection);
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

