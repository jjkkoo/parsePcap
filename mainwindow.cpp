
#include <QtWidgets>

#include "mainwindow.h"

MainWindow::MainWindow() : m_tempMediaFile(QList<QTemporaryFile *>()) ,
 m_tempDecodedFile(QVector<QTemporaryFile *>()), m_codecVector(QVector<int>())
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
    decodeThread = nullptr;

    initUI();
    readSettings();

    tableView->setAcceptDrops(false);
    chartView->setAcceptDrops(false);
    splitter->setAcceptDrops(false);
    setAcceptDrops(true);
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
    //qDebug() << QCoreApplication::applicationDirPath()<< "," << QDir::currentPath() << "," << QDir::current().path();
    qDebug() << pickDir;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (parseThread == nullptr and decodeThread == nullptr) {
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
                else if (decodeThread != nullptr and decodeThread->isRunning()) {
                    decodeThread->stopMe();
                }
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

    foreach (const QTemporaryFile * fPtr, m_tempDecodedFile)
        if(fPtr != nullptr) {delete fPtr;}
    m_tempDecodedFile.clear();

    writeSettings();
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

    m_tempDecodedFile.append(QVector<QTemporaryFile * >(fileNameList.size(), nullptr));
    m_codecVector.append(QVector<int>(fileNameList.size(), -1));
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
    if (parseThread != nullptr and parseThread->isRunning()){
        parseThread->stopMe();
    }
    if (decodeThread != nullptr and decodeThread->isRunning()){
        decodeThread->stopMe();
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
    m_pProgressBar->setValue(0);
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
    qDebug() << "clearFile action" << m_tempMediaFile.count() << m_tempDecodedFile.count();
    tableModel->clearData();

    if (m_tempMediaFile.count()) {
        foreach (const QTemporaryFile * fPtr, m_tempMediaFile)
            delete fPtr;
        m_tempMediaFile.clear();
    }
    if (m_tempDecodedFile.count()) {
        foreach (const QTemporaryFile * fPtr, m_tempDecodedFile)
            if(fPtr != nullptr) {delete fPtr;}
        m_tempDecodedFile.clear();
    }
    m_codecVector.clear();
    initUI();
}

void MainWindow::exit()
{
    qDebug() << "exit action";
    close();
}

void MainWindow::startDecoding(int index)
{
    m_pProgressBar->setValue(0);
    m_pProgressBar->show();
    decodeThread = new DecodeThread(m_tempMediaFile.at(index), tableModel->index(index, COL_codec).data().toInt(), this);
    connect(decodeThread, &DecodeThread::updateProgress, this, &MainWindow::refreshProgress);
    connect(decodeThread, &DecodeThread::decodeSuccess, this, &MainWindow::decodeFinished);
    connect(decodeThread, &DecodeThread::lastWords, this, &MainWindow::lastWords);
    connect(decodeThread, &DecodeThread::finished, this, &MainWindow::decodeThrOver);
    decodeThread->start();
    //qDebug() << QDateTime::currentDateTime();
}

void MainWindow::decodeFinished(QTemporaryFile * decodeResult)
{
    //qDebug() << QDateTime::currentDateTime();
    if (waitForPlotList.size() > 0) {
        m_tempDecodedFile[waitForPlotList.at(0)] = decodeResult;
        m_codecVector[waitForPlotList.at(0)] = tableModel->index(waitForPlotList.at(0), COL_codec).data().toInt();
        plotOnChart(waitForPlotList);
        waitForPlotList.clear();
    }
    else if (waitForExportList.size() > 0){
        m_tempDecodedFile[waitForExportList.at(0)] = decodeResult;
        m_codecVector[waitForExportList.at(0)] = tableModel->index(waitForExportList.at(0), COL_codec).data().toInt();
        exportAfterDecode(waitForExportList);
        waitForExportList.clear();
    }
}

void MainWindow::decodeThrOver()
{
    m_pProgressBar->hide();
    decodeThread->deleteLater();
    decodeThread = nullptr;
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

    if (m_tempDecodedFile.at(indexList[0]) == nullptr){
        waitForPlotList.append(indexList[0]);
        startDecoding(indexList[0]);
    }

    else {
        plotOnChart(indexList);
    }

}

void MainWindow::plotOnChart(QList<int>indexList)
{
    QFile mediaDataFile(m_tempDecodedFile.at(indexList.at(0))->fileName());
    if(!mediaDataFile.open(QIODevice::ReadOnly)) {
        statusBar()->showMessage("error reading cache files!");
        return;
    }
    double codec = (tableModel->index(indexList[0], COL_codec).data().toString() == "0") ? 1.0 / 8000 : 1.0 /16000;
    double maxLen = mediaDataFile.size()/2.0*codec;

    zoomInfo zi{0, maxLen, maxLen, maxLen};
    chartView->setZoomInfo(zi);

    QVector<QPointF> points;
    char shortData[2];
    int tempMax = 0, tempCounter = 0, pLen = mediaDataFile.size() / 2,
            threashHolder = pLen / 4000;
//        qDebug() << QDateTime::currentDateTime();
    for (int k = 0; k < pLen; ++k) {
        mediaDataFile.read(shortData, 2);
        if (++tempCounter > threashHolder) {
            points.append( QPointF(k * codec, *(short *)shortData ));
            if (qAbs(*(short *)shortData) > tempMax)
                tempMax = qAbs(*(short *)shortData);
            tempCounter = 0;
        }
    }
//        qDebug() << QDateTime::currentDateTime();
    chart->axisX(series)->setRange(0, mediaDataFile.size()/2.0*codec );
    chart->axisY(series)->setRange(-tempMax, tempMax );
   // qDebug() << points;
    series->replace(points);
    series->setName(QString("index:%1_%2_%3_%4_%5").arg(indexList[0] + 1).arg(tableModel->index(indexList[0], COL_source_ip).data().toString()).arg(tableModel->index(indexList[0], COL_srcPort).data().toString())
            .arg(tableModel->index(indexList[0], COL_dest_ip).data().toString()).arg(tableModel->index(indexList[0], COL_destPort).data().toString()));
    QPen pen(QRgb("#E6E6FA"));
    series->setPen(pen);
    chartView->show();
    mediaDataFile.close();
}

void MainWindow::exportMedia()
{
    qDebug() << "exportMedia action";
    QItemSelectionModel * selection = tableView->selectionModel();
    QModelIndexList indexes = selection->selectedIndexes();
    QSet<int> indexRowSet;
    foreach(const QModelIndex& index, indexes){
        indexRowSet.insert(index.row());
    }
    QList<int> indexList = indexRowSet.toList();
    //qSort(indexList.begin(), indexList.end());
    //qDebug() << "index:" << indexList[0] << "codec:" << tableModel->index(indexList[0], COL_codec).data().toInt() << m_tempMediaFile.at(indexList[0])->fileName();

   // if(indexList.size() != 0) {
    //    foreach ( const int & index, indexList ) {
            if (m_tempDecodedFile.at(indexList[0]) == nullptr) {
                waitForExportList.append(indexList[0]);
                startDecoding(indexList[0]);
            }
            else {
                saveMediaFile(indexList[0]);
            }
  //      }
   // }
}

void MainWindow::exportAfterDecode(QList<int> indexList)
{
    foreach ( const int & index, indexList ) {
        saveMediaFile(index);
    }
}

void MainWindow::saveMediaFile(int index)
{
    //qDebug() << tableModel->getLine(index).join("_").replace(":", " ");
    QFile file(QString("index%1_%2").arg(index + 1).arg(tableModel->getLine(index).join("_").replace(":", " ")));
    if (!file.open(QIODevice::WriteOnly))    return;
    qDebug() << file.fileName() << m_codecVector.at(index);

    quint32 sampleRate = (m_codecVector.at(index) == 0 or m_codecVector.at(index) == 2) ? 8000 : 16000;
    quint32 fileSize = m_tempDecodedFile.at(index)->size();
    CombinedHeader wavHeader { { {{'R','I','F','F'}, fileSize + 36}, {'W','A','V','E'} } , { {{'f','m','t',' '}, 16}, 1, 1,
            sampleRate, 2 * sampleRate, 2 ,16} };
    file.write( (char *) &wavHeader, sizeof(CombinedHeader));
    m_tempDecodedFile.at(index)->open();
    file.write(dataConst , 4);
    file.write((char *) &fileSize, 4);
    file.write(m_tempDecodedFile.at(index)->readAll());
    m_tempDecodedFile.at(index)->close();
    file.close();
}

void MainWindow::play()
{
    qDebug() << "play action";
}

void MainWindow::stop()
{
    qDebug() << "stop action";
}

void MainWindow::showPreference()
{
    qDebug() << "showPreference";
    TabDialog tabdialog(this);
    tabdialog.exec();
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

    preferenceAct = new QAction(tr("&Preference"), this);
    preferenceAct->setShortcut(tr("F10"));
    preferenceAct->setStatusTip(tr("Preference"));
    connect(preferenceAct, &QAction::triggered, this, &MainWindow::showPreference);

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

    settingsMenu= menuBar()->addMenu(tr("&Settings"));
    settingsMenu->addAction(preferenceAct);

    helpMenu = menuBar()->addMenu(tr("&Help"));
//    helpMenu->addAction(aboutAct);
//    helpMenu->addAction(aboutQtAct);

}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    //if (event->mimeData()->hasFormat("text/plain"))
    event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) {return;}

    QString fileName = urls.first().toLocalFile();
    qDebug() << fileName;
    startParsing(fileName);

}
