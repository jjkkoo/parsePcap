#include "comboboxdelegate.h"
#include "dialogmodel.h"

#ifndef TABDIALOG_H
#define TABDIALOG_H

#include <QDialog>
#include <QSettings>

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QTabWidget;
class QLineEdit;
class QTableview;
QT_END_NAMESPACE


class GeneralTab : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralTab( QWidget *parent = 0);
    ~GeneralTab();

private:
    QLineEdit *fileNameEdit;
};

class PayLoadTypeMapTab : public QWidget
{
    Q_OBJECT

public:
    explicit PayLoadTypeMapTab(QWidget *parent = 0);
    ~PayLoadTypeMapTab();
private:
    dialogModel * tableModel;
};

class TabDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TabDialog(QWidget *parent = 0);
    ~TabDialog();

private slots:
    void nextTab();

private:
    QTabWidget *tabWidget;
    QDialogButtonBox *buttonBox;
};

#endif
