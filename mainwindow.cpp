
#include <QtWidgets>

#include "mainwindow.h"

MainWindow::MainWindow() : m_tempMediaFile(QList<QTemporaryFile *>()) ,
    m_tempDecodedFile(QVector<QTemporaryFile *>()), m_codecVector(QVector<int>()),
    currentPlotIndex(-1), m_pushTimer(new QTimer(this))
{
    /* create MainWindow ui widgets */
    QWidget *widget = new QWidget;
    setCentralWidget(widget);

    tableView = new CustomTableView;
    //tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableView->resizeColumnsToContents();
    tableView->horizontalHeader()->setStretchLastSection(true);

    tableModel = new CustomTableModel;
    tableView->setModel(tableModel);

    ComboBoxDelegate* delegate = new ComboBoxDelegate;
    tableView->setItemDelegateForColumn(COL_codec, delegate);

    chart = new QChart;
    chart->setAnimationOptions(QChart::AllAnimations);

    series = new QLineSeries;

    chart->addSeries(series);
    chart->createDefaultAxes();

    QValueAxis *axisX = new QValueAxis;
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
    connect(chartView, &ChartView::updateStartPosFromClick, this, &MainWindow::updatePlayfilePos);
    connect(chartView, &ChartView::showAllOneSecondPoints, this, &MainWindow::showAllOneSecondPoints);
    connect(chartView, &ChartView::showAllSampledPoints, this, &MainWindow::showAllSampledPoints);

    splitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(tableView);
    splitter->addWidget(chartView);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(splitter);
    widget->setLayout(layout);

    m_pProgressBar = new QProgressBar;
    statusBar()->addWidget(m_pProgressBar);

    createActions();
    createMenus();

    /* initiate MainWindow ui vars */
    initUI();
    setupPosition();
    tableView->setAcceptDrops(false);
    chartView->setAcceptDrops(false);
    splitter->setAcceptDrops(false);
    setAcceptDrops(true);

    QDir dir;
    if (dir.mkdir("temp"))    qDebug() << "no temp dir, creating one";
    removeTempFile();
}

void MainWindow::initUI()
{
    m_pProgressBar->hide();
    //chartView->hide();    //buggy
    splitter->setSizes(QList<int>{splitter->height(), 0});
    statusBar()->showMessage("");
}

void MainWindow::savePosition()
{
    QSettings settings("jjkkoo", "parsePcap");
    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();
}

void MainWindow::setupPosition()
{
    QSettings settings("jjkkoo", "parsePcap");
    settings.beginGroup("MainWindow");
    resize(settings.value("size", QSize(798, 507)).toSize());
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    settings.endGroup();
    //qDebug() << QCoreApplication::applicationDirPath()<< "," << QDir::currentPath() << "," << QDir::current().path();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    stop();
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

    savePosition();
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
    for (int i = 0; i < tableModel->columnCount() - 1; ++i){
        tableView->resizeColumnToContents(i);
    }
    tableView->scrollToBottom();
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
    QSettings settings("jjkkoo", "parsePcap");
    settings.beginGroup("MainWindow");
    QString pickDir = settings.value("pickDirectory", QCoreApplication::applicationDirPath()).toString();

    QStringList pathList = QFileDialog::getOpenFileNames(this, tr("Pick Pcap Files"), pickDir, tr("Pcap Files(*.pcap);;All Files(*.*)"));
    qDebug() << pathList;
    if(pathList.isEmpty()) {
        statusBar()->showMessage("Pick Pcap Files Action Cancelled");
        return;
    }

    foreach (const QString &str, pathList)
        startParsing(str);
    settings.setValue("pickDirectory", QFileInfo(pathList[0]).absolutePath());
    settings.endGroup();
}

void MainWindow::clearTable()
{
    if (audio != nullptr) {
        audio->stop();
        audio = nullptr;
    }
    currentPlotIndex = -1;
    PlayerFile = nullptr;
    currentSampleRate = 0;
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
    tableModel->clearMarked();
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
    if (tableModel->index(index, COL_codec).data().toInt() > 3) {
        statusBar()->showMessage("amr or amr_wb only for now!");
        return;
    }
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
        //waitForPlotList.clear();
    }
    else if (waitForExportList.size() > 0){
        m_tempDecodedFile[waitForExportList.at(0)] = decodeResult;
        m_codecVector[waitForExportList.at(0)] = tableModel->index(waitForExportList.at(0), COL_codec).data().toInt();
        exportAfterDecode(waitForExportList);
        //waitForExportList.clear();
    }
}

void MainWindow::decodeThrOver()
{
    m_pProgressBar->hide();
    decodeThread->deleteLater();
    decodeThread = nullptr;
    waitForPlotList.clear();
    waitForExportList.clear();
}

void MainWindow::plot()
{
    qDebug() << "plot action";
    QItemSelectionModel * selection = tableView->selectionModel();
    QModelIndexList indexes = selection->selectedIndexes();
    if (indexes.size() <= 0)    return;
    QSet<int> indexRowSet;
    foreach(const QModelIndex& index, indexes){
        indexRowSet.insert(index.row());
    }
    QList<int> indexList = indexRowSet.toList();
    qSort(indexList.begin(), indexList.end());
    qDebug() << "index:" << indexList[0] << "codec:" << tableModel->index(indexList[0], COL_codec).data().toInt() << m_tempMediaFile.at(indexList[0])->fileName();

        if (m_tempDecodedFile.at(indexList[0]) == nullptr or
            m_codecVector.at(indexList[0]) != tableModel->index(indexList[0], COL_codec).data().toInt()) {
            waitForPlotList.append(indexList[0]);
            startDecoding(indexList[0]);
        }
        else
            plotOnChart(indexList);
}

void MainWindow::plotOnChart(QList<int>indexList)
{
    QFile mediaDataFile(m_tempDecodedFile.at(indexList.at(0))->fileName());
    if(!mediaDataFile.open(QIODevice::ReadOnly)) {
        statusBar()->showMessage("error reading cache files!");
        return;
    }
    if (waitForPlotList.empty() and currentPlotIndex == indexList[0]) {    //reset chartView
        zoomInfo zi = chartView->getZoomInfo(0);
        zoomInfo newZi{0, zi.max, zi.max, zi.max};
        chartView->setZoomInfo(newZi);
        showAllSampledPoints();
        if (newZi.step < 0.05)
            series->setPointsVisible(true);
        else
            series->setPointsVisible(false);
        chart->axisX(series)->setRange(0, newZi.end);
    }
    else {
        double codec = (tableModel->index(indexList[0], COL_codec).data().toString() == "0") ? 1.0 / 8000 : 1.0 /16000;
        double maxLen = mediaDataFile.size()/2.0*codec;
        zoomInfo zi{0, maxLen, maxLen, maxLen};
        chartView->setZoomInfo(zi);

        QVector<QPointF> points;
        char shortData[2];
        unsigned int tempCounter = 0, pLen = mediaDataFile.size() / 2, threashHolder = pLen / 16000;
        int tempMax = 0;
        for (unsigned int k = 0; k < pLen; ++k) {
            mediaDataFile.read(shortData, 2);
            if (++tempCounter > threashHolder) {
                points.append( QPointF(k * codec, *(short *)shortData ));
                if (qAbs(*(short *)shortData) > tempMax)
                    tempMax = qAbs(*(short *)shortData);
                tempCounter = 0;
            }
        }
        chart->axisX(series)->setRange(0, maxLen );
        if (tempMax > 0)    // setRange(-0, 0) will crash;
            chart->axisY(series)->setRange(-tempMax, tempMax );
        series->replace(points);
        series->setName(QString("index:%1_%2_%3_%4_%5").arg(indexList[0] + 1).arg(tableModel->index(indexList[0], COL_source_ip).data().toString()).arg(tableModel->index(indexList[0], COL_srcPort).data().toString())
                .arg(tableModel->index(indexList[0], COL_dest_ip).data().toString()).arg(tableModel->index(indexList[0], COL_destPort).data().toString()));
        QPen pen(QColor("#607B8B"));    //LightSkyBlue4: #607B8B    #E6E6FA: lavender
        series->setPen(pen);

        if (audio != nullptr) {
            audio->stop();
            audio = nullptr;
        }

        if (zi.step < 0.05)
            series->setPointsVisible(true);
        else
            series->setPointsVisible(false);

        chartView->refreshProgress(0);
        chartView->resizeProgressLine();
        splitter->setSizes(QList<int>{splitter->height() / 2, splitter->height() / 2});
        mediaDataFile.close();
        currentPlotIndex = indexList.at(0);
        PlayerFile = m_tempDecodedFile.at(indexList.at(0));
        currentFileSize = PlayerFile->size();
        currentPlayPos = 0;
        startPlayPos = 0;
        switch (m_codecVector.at(indexList.at(0))) {
            case 0:
            case 2:    currentSampleRate = 8000;    break;
            case 1:
            case 3:    currentSampleRate = 16000;   break;
            default:   currentSampleRate = 8000;
        }
    }
}

void MainWindow::showAllOneSecondPoints()
{
    QFile mediaDataFile(PlayerFile->fileName());
    if(!mediaDataFile.open(QIODevice::ReadOnly)) {
        statusBar()->showMessage("error reading cache files!");
        return;
    }
    QVector<QPointF> points;
    char shortData[2];
    zoomInfo z = chartView->getZoomInfo(0);
    qint64 startPos = z.start * currentSampleRate * 2;
    if (startPos % 2 !=0)    --startPos;
    mediaDataFile.seek(startPos);
    double codec = 1.0 / currentSampleRate;
    bool dataAvailable = true;
    for (unsigned int k = 0; k <= z.step * currentSampleRate; ++k) {
        if (dataAvailable) {
            qint64 ret = mediaDataFile.read(shortData, 2);
            if (ret < 2)    dataAvailable = false;
            points.append( QPointF(z.start + k * codec, *(short *)shortData ));
        }
        else {
            points.append( QPointF(k * codec, 0));
        }
    }
    series->replace(points);
}

void MainWindow::showAllSampledPoints()
{
    QFile mediaDataFile(PlayerFile->fileName());
    if(!mediaDataFile.open(QIODevice::ReadOnly)) {
        statusBar()->showMessage("error reading cache files!");
        return;
    }
    unsigned int tempCounter = 0, pLen = mediaDataFile.size() / 2, threashHolder = pLen / 16000;
    QVector<QPointF> points;
    char shortData[2];
    double codec = 1.0 / currentSampleRate;

    for (unsigned int k = 0; k < pLen; ++k) {
        mediaDataFile.read(shortData, 2);
        if (++tempCounter > threashHolder) {
            points.append( QPointF(k * codec, *(short *)shortData ));
            tempCounter = 0;
        }
    }
    series->replace(points);
}

void MainWindow::exportMedia()
{
    qDebug() << "exportMedia action";
    QItemSelectionModel * selection = tableView->selectionModel();
    QModelIndexList indexes = selection->selectedIndexes();
    if (indexes.size() <= 0)    return;
    QSet<int> indexRowSet;
    foreach(const QModelIndex& index, indexes){
        indexRowSet.insert(index.row());
    }
    QList<int> indexList = indexRowSet.toList();
    //qSort(indexList.begin(), indexList.end());
    //qDebug() << "index:" << indexList[0] << "codec:" << tableModel->index(indexList[0], COL_codec).data().toInt() << m_tempMediaFile.at(indexList[0])->fileName();

   // if(indexList.size() != 0) {
    //    foreach ( const int & index, indexList ) {
            if (m_tempDecodedFile.at(indexList[0]) == nullptr or
                    m_codecVector.at(indexList[0]) != tableModel->index(indexList[0], COL_codec).data().toInt()) {
                waitForExportList.append(indexList[0]);
                startDecoding(indexList[0]);
            }
            else {
                exportAfterDecode(indexList);
            }
  //      }
   // }

}

void MainWindow::exportAfterDecode(QList<int> indexList)
{
    QSettings settings("jjkkoo", "parsePcap");

    settings.beginGroup("MainWindow");
    QString saveDir = settings.value("saveDirectory", QCoreApplication::applicationDirPath()).toString();

    QString directory = QFileDialog::getExistingDirectory(this, "select export directory", saveDir);
    if (directory.isEmpty())    return;
    //qDebug() << directory << directory.isEmpty();

    //foreach ( const int & index, indexList ) {
        saveMediaFile(indexList[0], directory);
    //}

    settings.setValue("saveDirectory", directory);
    settings.endGroup();
}

void MainWindow::saveMediaFile(int index, QString directory)
{
    //qDebug() << tableModel->getLine(index).join("_").replace(":", " ");
    QString absFilePath = QString("%1//index%2_%3.wav").arg(directory).arg(index + 1).arg(tableModel->getLine(index).join("_").replace(":", " "));
    QFile file(absFilePath);
    if (!file.open(QIODevice::WriteOnly))    return;
    //qDebug() << file.fileName() << m_codecVector.at(index);

    quint32 sampleRate;
    switch (m_codecVector.at(index)) {
        case 0:
        case 2:    sampleRate = 8000;    break;
        case 1:
        case 3:    sampleRate = 16000;   break;
        default:   sampleRate = 8000;
    }
    quint32 fileSize = m_tempDecodedFile.at(index)->size();
    CombinedHeader wavHeader { { {{'R','I','F','F'}, fileSize + 36}, {'W','A','V','E'} } , { {{'f','m','t',' '}, 16}, 1, 1, sampleRate, 2 * sampleRate, 2 ,16} };

    file.write( (char *) &wavHeader, sizeof(CombinedHeader));
    m_tempDecodedFile.at(index)->open();
    file.write(dataConst , 4);
    file.write((char *) &fileSize, 4);
    file.write(m_tempDecodedFile.at(index)->readAll());
    m_tempDecodedFile.at(index)->close();
    file.close();
    statusBar()->showMessage(QString("index%1: %2").arg(index+1).arg(absFilePath));
}

void MainWindow::play()
{
    qDebug() << "play action";
    if (PlayerFile == nullptr)    return;
    if (audio!=nullptr and audio->state() == QAudio::ActiveState) {
        audio->suspend();
        m_pushTimer->stop();
    }
    else if (audio!=nullptr and audio->state() == QAudio::SuspendedState) {
        audio->resume();
        m_pushTimer->start();
    }
    else {
        PlayerFile->close();
        PlayerFile->open();

        QAudioFormat format;
        format.setSampleRate(currentSampleRate);
        format.setChannelCount(1);
        format.setSampleSize(16);
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setSampleType(QAudioFormat::UnSignedInt);//?

         QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
        if (!info.isFormatSupported(format)) {
            qWarning() << "Raw audio format not supported by backend, cannot play audio.";
            return;
        }

        if (audio != nullptr)    delete audio;    //no setFormat ??
//        audio = new QAudioOutput( format, this);
//        connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
//        audio->setNotifyInterval(150);
//        connect(audio, SIGNAL(notify()), this, SLOT(playerRefreshProgress()));
//        audio->start(PlayerFile);

        audio = new QAudioOutput( format, this);
        connect(m_pushTimer, SIGNAL(timeout()), this, SLOT(pushTimerExpired()));
        if (!(startPlayPos > currentFileSize)){// or startPlayPos < 0)){
            PlayerFile->seek(startPlayPos);
            currentPlayPos = startPlayPos;
        }
        m_output = audio->start();
        connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
        m_pushTimer->start(20);
    }
}

void MainWindow::pushTimerExpired()
{
    if (audio && audio->state() != QAudio::StoppedState && audio->bytesFree()) {
        //qDebug() <<"bytesFree:"<< audio->bytesFree() << " periodSize:" << audio->periodSize();
        int chunks = audio->bytesFree()/audio->periodSize();
        if (chunks == 0)    return;
        currentPlayPos += (unsigned int)(chunks * audio->periodSize());
//        qDebug() << chunks;
//        qDebug() << 1.0 * currentPlayPos / currentFileSize;
        chartView->refreshProgress(1.0 * currentPlayPos / currentFileSize );
        while (chunks) {
           const qint64 len = PlayerFile->read(m_buffer, audio->periodSize());
           if (len) {
               if (m_output->write(m_buffer, len) != len) {
                   m_pushTimer->stop();
                   return;
               }
           }
           --chunks;
        }
        if (currentPlayPos >= currentFileSize) {
            chartView->finishProgress();
            currentPlayPos = 0;
            m_pushTimer->stop();
            audio->stop();
            PlayerFile->close();
            delete audio;
            audio = nullptr;
        }
    }
}

void MainWindow::updatePlayfilePos(double posPercent)
{
    currentPlayPos = posPercent * currentFileSize;
    if(currentPlayPos % 2 != 0)    currentPlayPos -= 1;
    startPlayPos = currentPlayPos;
    if(PlayerFile->isOpen())
        PlayerFile->seek(currentPlayPos);
}

void MainWindow::handleStateChanged(QAudio::State newState)
{
    switch (newState) {
    /*
        case QAudio::IdleState:
            // Finished playing (no more data)
            audio->stop();
            PlayerFile->close();
            delete audio;
            audio = nullptr;
            m_pushTimer->stop();
            break;
    */
        case QAudio::StoppedState:
            // Stopped for other reasons
            if (audio->error() != QAudio::NoError) {
                PlayerFile->close();
                delete audio;
                audio = nullptr;
            }
            m_pushTimer->stop();
            break;

        default:
            // ... other cases as appropriate
            break;
    }
}

//void MainWindow::playerRefreshProgress()
//{
//    //qDebug() << audio->processedUSecs();
//    //qDebug() << audio->periodSize();
//    //qDebug() << 1.0 * audio->processedUSecs() / 1000000 / (currentFileSize / (2 * 16000));
//    chartView->refreshProgress(1.0 * audio->processedUSecs() / 1000000 / (currentFileSize / (2 * currentSampleRate)));
//}

void MainWindow::stop()
{
    qDebug() << "stop action";
    if (audio == nullptr)    return;
    audio->stop();
    PlayerFile->close();
    delete audio;
    audio = nullptr;
}

void MainWindow::showPreference()
{
    qDebug() << "showPreference";
    TabDialog tabdialog(this);
    tabdialog.exec();
}

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

    toggleMarkAct = new QAction(tr("&Mark/UnMark"), this);
    toggleMarkAct->setShortcut(tr("Ctrl+F2"));
    toggleMarkAct->setStatusTip(tr("Mark/UnMark"));
    connect(toggleMarkAct, &QAction::triggered, this, &MainWindow::toggleMark);

    seekMarkAct = new QAction(tr("&Jump To Mark"), this);
    seekMarkAct->setShortcut(tr("F2"));
    seekMarkAct->setStatusTip(tr("Jump To Mark"));
    connect(seekMarkAct, &QAction::triggered, this, &MainWindow::seekMark);

    QShortcut * escShortcut = new QShortcut(QKeySequence::Cancel, this);
    connect(escShortcut, &QShortcut::activated, this, &MainWindow::cancelAll);
    //connect(escShortcut, SIGNAL(activated()), this, SLOT(cancelAll()));

    QShortcut * copyShortcut = new QShortcut(QKeySequence::Copy, this);
    connect(copyShortcut, &QShortcut::activated, this, &MainWindow::copySelection);

    QShortcut * selectAllShortcut = new QShortcut( QKeySequence(tr("Ctrl+A")), this);
    connect(selectAllShortcut, &QShortcut::activated, this, &MainWindow::selectAll);
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
    editMenu->addSeparator();
    editMenu->addAction(toggleMarkAct);
    editMenu->addAction(seekMarkAct);
    editMenu->addSeparator();

    playMenu = editMenu->addMenu(tr("&Play"));
    playMenu->addAction(playAct);
    playMenu->addAction(stopAct);

    modesMenu= menuBar()->addMenu(tr("&Modes"));

    settingsMenu = menuBar()->addMenu(tr("&Settings"));
    settingsMenu->addAction(preferenceAct);

    helpMenu = menuBar()->addMenu(tr("&Help"));
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

    QSettings settings("jjkkoo", "parsePcap");
    settings.beginGroup("MainWindow");
    settings.setValue("pickDirectory", QFileInfo(fileName).absolutePath());
    settings.endGroup();
}

void MainWindow::removeTempFile()
{
    QDir dir("temp");
    foreach (QFileInfo fileInfo, dir.entryInfoList(QDir::Files)) {
//        int ret = remove(qPrintable(fileInfo.fileName()));
//        qDebug() << ret << " : " << fileInfo.fileName();
        //QLockFile lockFile(fileInfo.fileName());
        //qDebug() << lockFile.isLocked();
        //file.setPermissions(QFile::WriteOwner);
        QFile file("temp\\" + fileInfo.fileName());
        if(!file.remove()) {
            qDebug() << "can not remove file: " << fileInfo.fileName();
        }
    }
}

void MainWindow::toggleMark()
{
    qDebug() << "toggleMark";
    QItemSelectionModel * selection = tableView->selectionModel();
    QModelIndexList indexes = selection->selectedIndexes();
    if (indexes.size() <= 0)    return;
    QSet<int> indexRowSet;
    foreach(const QModelIndex& index, indexes){
        indexRowSet.insert(index.row());
    }
    tableModel->toggleMarkUnmark(indexRowSet);
}

void MainWindow::seekMark()
{
    qDebug() << "seekMark";
    std::set<int> targetSet = tableModel->getMarked();
    if (targetSet.empty())    return;    //if marked list is empty, do nothing

    QItemSelectionModel * selection = tableView->selectionModel();
    QModelIndexList indexes = selection->selectedIndexes();

    int tempMax(-1);
    std::set<int>::iterator it;
    if (indexes.size() <= 0) {    //if nothing is selected
        it = targetSet.begin();
    }
    else {                        //else seed max row number, and get upperBound or begin
        foreach(const QModelIndex& index, indexes){
            if (index.row() > tempMax)
                tempMax = index.row();
        }
        it = targetSet.upper_bound(tempMax);
        if( it == targetSet.end())    it = targetSet.begin();
    }
    selection->select(QItemSelection(tableView->model()->index(*it, 0),
                                     tableView->model()->index(*it, columnHeader.size() - 1)),
                      QItemSelectionModel::ClearAndSelect);
    tableView->scrollTo(tableView->model()->index(*it, 0));

}

void MainWindow::selectAll()
{
    qDebug() << "selectAll";
    QItemSelectionModel * selection = tableView->selectionModel();
    selection->select(QItemSelection(tableView->model()->index(0, 0),
                                     tableView->model()->index(tableModel->rowCount() - 1, columnHeader.size() - 1)),
                      QItemSelectionModel::ClearAndSelect);
}

