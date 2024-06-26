#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include <glib.h>
#include <gio/gio.h>

#include <QMainWindow>

#include <QShortcut>
#include <QMenuBar>
#include <QMenu>

//#include "rotator.h"

class BaseWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BaseWindow(QWidget *parent = 0);

private:
    //void closeChildren();

private slots:
    void showWindowMenu();
};

#endif // BASEWINDOW_H
