#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include "mafw/mafwregistryadapter.h"

#include "basewindow.h"

#include <QTimer>
#include <QScrollBar>

#include "ui_browserwindow.h"
#include "includes.h"
#include "rotator.h"
#include "headerawareproxymodel.h"

namespace Ui {
    class BrowserWindow;
}

class BrowserWindow : public BaseWindow
{
    Q_OBJECT

public:
    explicit BrowserWindow(QWidget *parent, MafwRegistryAdapter *mafwRegistry);
    ~BrowserWindow();

    bool eventFilter(QObject *, QEvent *e);

protected:
    Ui::BrowserWindow *ui;

    QStandardItemModel *objectModel;
    QSortFilterProxyModel *objectProxyModel;

    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

protected Q_SLOTS:
    void orientationInit();
    void onOrientationChanged(int w, int h);
    void onSearchRequested();
    void onSearchHideButtonClicked();
    void onSearchTextChanged(QString);
    void onChildClosed();
};

#endif // BROWSERWINDOW_H
