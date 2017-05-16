
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

    chart = new Chart;
    chart->setAnimationOptions(Chart::AllAnimations);

    series = new QLineSeries;

    chart->addSeries(series);
    chart->createDefaultAxes();

    QValueAxis *axisX = new QValueAxis;
    //axisX->setRange(0, 2000);
    axisX->setLabelFormat("%.3f");
    axisX->setTitleText("Reference Time");
    QValueAxis *axisY = new QValueAxis;
    axisY->setRange(-32768, 32768);
    axisY->setLabelFormat("%d");
    axisY->setTitleText("Audio level");
    chart->setAxisX(axisX, series);
    chart->setAxisY(axisY, series);
    //chart->legend()->hide();
    chart->setTitle("Time Domain Plotting");
    chart->setTheme(QChart::ChartThemeLight);

    chartView = new ChartView(chart);

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

    foreach (const QTemporaryFile * fPtr, m_tempMediaFile)
        delete fPtr;
    m_tempMediaFile.clear();
}

void MainWindow::refreshProgress(int value)
{
    m_pProgressBar->setValue(value);
}

void MainWindow::parseFinished(QList<QStringList> parsedList,  QList<QTemporaryFile *> fileNameList)//QStringList fileNameList)
{
    int beforeAppendRowCount = tableModel->rowCount();
    tableModel->appendData(parsedList);

    for ( int i = beforeAppendRowCount; i < tableModel->rowCount(); ++i ) {
        tableView->openPersistentEditor( tableModel->index(i, COL_codec) );
    }
    tableView->resizeColumnsToContents();

    m_tempMediaFile.append(fileNameList);
}

void MainWindow::lastWords(const QString &laswords) {
    statusBar()->showMessage(laswords);
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
    connect(parseThread, &ParseThread::lastWords, this, &MainWindow::lastWords);
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

void MainWindow::clearTable()
{
    qDebug() << "clearFile action";
    tableModel->clearData();

    foreach (const QTemporaryFile * fPtr, m_tempMediaFile)
        delete fPtr;
    m_tempMediaFile.clear();
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
    QSet<int> indexRowSet;
    foreach(const QModelIndex& index, indexes){
        indexRowSet.insert(index.row());
    }
    QList<int> indexList = indexRowSet.toList();
    qSort(indexList.begin(), indexList.end());
    qDebug() << "index:" << indexList[0] << "codec:" << tableModel->index(indexList[0], COL_codec).data().toInt() << m_tempMediaFile.at(indexList[0])->fileName();

    QFile mediaDataFile(m_tempMediaFile.at(indexList[0])->fileName());
    if(!mediaDataFile.open(QIODevice::ReadOnly)) {
        statusBar()->showMessage("error reading cache files!");
        return;
    }

    chart->axisX(series)->setRange(0, mediaDataFile.size()/2*0.05 );
    zoomInfo zi{0, mediaDataFile.size()/2*0.05, mediaDataFile.size()/2*0.05, mediaDataFile.size()/2*0.05};
    chartView->setZoomInfo(zi);

    QVector<QPointF> points;
    char shortData[2];
    for (int k = 0; k < mediaDataFile.size() / 2; ++k) {
        mediaDataFile.read(shortData, 2);
        points.append( QPointF(k * 0.05, *(short *)shortData ));
    }
   // qDebug() << points;
    series->replace(points);
    series->setName(QString("index:%1_%2_%3_%4_%5").arg(indexList[0] + 1).arg(tableModel->index(indexList[0], COL_source_ip).data().toString()).arg(tableModel->index(indexList[0], COL_srcPort).data().toString())
            .arg(tableModel->index(indexList[0], COL_dest_ip).data().toString()).arg(tableModel->index(indexList[0], COL_destPort).data().toString()));
//    QPen pen(QRgb(0x000000));
//    series->setPen(pen);
    chartView->show();
    mediaDataFile.close();
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
    connect(clearAct, &QAction::triggered, this, &MainWindow::clearTable);

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

