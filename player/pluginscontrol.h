#ifndef PLUGINSCONTROL_H
#define PLUGINSCONTROL_H

#include <QAction>

class PluginsControl : public QAction
{
    Q_OBJECT

public:
    PluginsControl(QWidget *parent);

Q_SIGNALS:
    void childOpened();
    void childClosed();

private:
    QHash<QString,QString> sources;

private Q_SLOTS:
    void onSourceAdded(const QString &uuid, const QString &name);
    void onSourceRemoved(const QString &uuid);
    void openWindow();
    void onChildClosed();
};

#endif // PLUGINSCONTROL_H
