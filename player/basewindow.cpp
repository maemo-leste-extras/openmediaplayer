#include "basewindow.h"

BaseWindow::BaseWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setAttribute(Qt::WA_Maemo5StackedWindow);

    connect(new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Enter), this), SIGNAL(activated()), this, SLOT(showWindowMenu()));
}

void BaseWindow::showWindowMenu()
{
    QMenu *menu = this->menuBar()->findChildren<QMenu*>().first();
    menu->adjustSize();
    int x = (this->width() - menu->width()) / 2;
    menu->exec(this->mapToGlobal(QPoint(x,-35)));
}
