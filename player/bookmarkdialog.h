#ifndef BOOKMARKDIALOG_H
#define BOOKMARKDIALOG_H

#include "mafw/mafwregistryadapter.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QMaemo5InformationBox>

#include "ui_bookmarkdialog.h"
#include "includes.h"
#include "rotator.h"

namespace Ui {
    class BookmarkDialog;
}

class BookmarkDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BookmarkDialog(QWidget *parent = 0, MafwRegistryAdapter *mafwRegistry = 0, Media::Type type = Media::Audio, QString address = "", QString name = "", QString objectId = "");
    ~BookmarkDialog();

private:
    Ui::BookmarkDialog *ui;
    MafwSourceAdapter *mafwRadioSource;
    QString objectId;

private Q_SLOTS:
    void accept();
    void onOrientationChanged(int h, int w);
};

#endif // BOOKMARKDIALOG_H
