#ifndef UPNPCONTROL_H
#define UPNPCONTROL_H

#include "mafw/mafwregistryadapter.h"

#include <QListWidget>

#include "includes.h"
#include "upnpview.h"

namespace Ui {
    class UpnpControl;
}

class UpnpControl : public QListWidget
{
    Q_OBJECT

public:
    explicit UpnpControl(QWidget *parent);

    void setRegistry(MafwRegistryAdapter *mafwRegistry);

Q_SIGNALS:
    void childOpened();
    void childClosed();

private Q_SLOTS:
    void onSourceAdded(const QString &uuid, const QString &name);
    void onSourceRemoved(const QString &uuid);
    void onItemActivated(QListWidgetItem *item);
    void onChildClosed();

private:
    MafwRegistryAdapter *mafwRegistry;
    MafwSourceAdapter *mafwUpnpSource;
    QStringList sources;
};

#endif // UPNPCONTROL_H
