#include "basewindow.h"

BaseWindow::BaseWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setProperty("X-Maemo-StackedWindow", 1);

    connect(new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Enter), this), SIGNAL(activated()), this, SLOT(showWindowMenu()));
}

void BaseWindow::showWindowMenu()
{
    QMenu *menu = this->menuBar()->findChildren<QMenu*>().first();
    menu->adjustSize();
    int x = (this->width() - menu->width()) / 2;
    menu->exec(this->mapToGlobal(QPoint(x,-35)));
}
