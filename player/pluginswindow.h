#ifndef PLUGINSWINDOW_H
#define PLUGINSWINDOW_H

#include "browserwindow.h"

class PluginsWindow : public BrowserWindow
{
    Q_OBJECT

public:
    PluginsWindow(QWidget *parent, const QHash<QString,QString> &sources);

private Q_SLOTS:
    void onItemActivated(const QModelIndex &index);
};

#endif // PLUGINSWINDOW_H
