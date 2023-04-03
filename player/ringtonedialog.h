#ifndef RINGTONEDIALOG_H
#define RINGTONEDIALOG_H

#include "mafw/mafwsourceadapter.h"

#include "confirmdialog.h"

#include <QDBusInterface>
#include <QUrl>
#include <QMaemo5InformationBox>

class RingtoneDialog : public ConfirmDialog
{
    Q_OBJECT

public:
    explicit RingtoneDialog(QWidget *parent,
                            MafwSourceAdapter *mafwSource, QString objectId,
                            QString title, QString artist);

private:
    bool accepted;
    QString objectId;
    QString uri;

    void setRingtone();

private Q_SLOTS:
    void done(int r);
    void onUriReceived(QString objectId, QString uri);
};

#endif // RINGTONEDIALOG_H
