#include <QtWidgets>
#include "tabdialog.h"

TabDialog::TabDialog(QWidget *parent)
    : QDialog(parent)
{
    QSettings settings("jjkkoo", "parsePcap");

    settings.beginGroup("tabDialogWindow");
    resize(settings.value("size", QSize(798, 507)).toSize());
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    settings.endGroup();

    tabWidget = new QTabWidget;
    tabWidget->addTab(new GeneralTab(), tr("General"));
    tabWidget->addTab(new PayLoadTypeMapTab(), tr("Payload Type"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);

    setLayout(mainLayout);

    setWindowTitle(tr("Preference"));

    QShortcut * tabShortcut = new QShortcut(QKeySequence(tr("tab")), this);
    connect(tabShortcut, &QShortcut::activated, this, &TabDialog::nextTab);

}

void TabDialog::nextTab()
{
    tabWidget->setCurrentIndex((tabWidget->currentIndex() + 1) % tabWidget->count());
}

TabDialog::~TabDialog(){
    QSettings settings("jjkkoo", "parsePcap");
    settings.beginGroup("tabDialogWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();
}

GeneralTab::GeneralTab(QWidget *parent)
    : QWidget(parent)
{
    QSettings settings("jjkkoo", "parsePcap");
    settings.beginGroup("MainWindow");
    QString filterString = settings.value("filterString", "ip and udp and udp[8]=0x80 or ip6 and udp").toString();
    settings.endGroup();

    QLabel *fileNameLabel = new QLabel(tr("pkt filter: appropriate filter could make parsing very efficient"));
    fileNameEdit = new QLineEdit(filterString);

    QLabel *bpsLinK = new QLabel(tr("<a href=\"https://www.winpcap.org/docs/docs_40_2/html/group__language.html\">Filtering expression syntax"));
    bpsLinK->setOpenExternalLinks(true);

    QLabel * egLabe = new QLabel(tr("default:    ip and udp and udp[8]=0x80 or ip6 and udp\nuniversal: ip or ip6\nspecific:    ip and udp src port 31038"
                                    "\n                 ip src host 10.10.73.8"));
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(fileNameLabel);
    mainLayout->addWidget(fileNameEdit);
    mainLayout->addWidget(bpsLinK);
    mainLayout->addWidget(egLabe);

    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

GeneralTab::~GeneralTab() {
    QSettings settings("jjkkoo", "parsePcap");
    settings.beginGroup("MainWindow");
    settings.setValue("filterString", fileNameEdit->text());
    settings.endGroup();
}

PayLoadTypeMapTab::PayLoadTypeMapTab(QWidget *parent)
    : QWidget(parent)
{

    QTableView *tableView = new QTableView;
    ComboBoxDelegate* delegate = new ComboBoxDelegate;
    tableModel = new dialogModel;
    tableView->setItemDelegateForColumn(1, delegate);
    tableView->setModel(tableModel);
    tableView->setColumnWidth(1, 150);

    QSettings settings("jjkkoo", "parsePcap");
    settings.beginGroup("PayLoadTypeMap");
    for(int i = 0; i < 32; ++i){
        int tempPtValue = settings.value(QString("pt%1").arg(i), 0).toInt();
        QModelIndex index= tableModel->index(i, 1, QModelIndex());
        tableModel->setData(index, tempPtValue);
    }
    settings.endGroup();

    for (int i = 0; i < tableModel->rowCount(); ++i)
        tableView->openPersistentEditor( tableModel->index(i, 1) );

    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(tableView);
    //mainLayout->addStretch(1);
    setLayout(mainLayout);
}

PayLoadTypeMapTab::~PayLoadTypeMapTab()
{
    QSettings settings("jjkkoo", "parsePcap");
    settings.beginGroup("PayLoadTypeMap");
    for(int i = 0; i < 32; ++i){
        QModelIndex index= tableModel->index(i, 1, QModelIndex());
        settings.setValue(QString("pt%1").arg(i), tableModel->data(index));
    }
    settings.endGroup();
}
