#ifndef PLAYLISTPICKER_H
#define PLAYLISTPICKER_H

#include "mafw/mafwplaylistadapter.h"
#include "mafw/mafwplaylistmanageradapter.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QKeyEvent>

#include "ui_playlistpicker.h"
#include "includes.h"
#include "confirmdialog.h"

namespace Ui {
    class PlaylistPicker;
}

class PlaylistPicker : public QDialog
{
    Q_OBJECT

public:
    explicit PlaylistPicker(QWidget *parent = 0);
    ~PlaylistPicker();

    QString playlistName;

private:
    Ui::PlaylistPicker *ui;

    void keyPressEvent(QKeyEvent *e);

    QDialog *createPlaylistDialog;
    QLineEdit *playlistNameEdit;

private Q_SLOTS:
    void onCreatePlaylist();
    void onCreatePlaylistAccepted();
    void onItemActivated(QListWidgetItem *item);
};

#endif // PLAYLISTPICKER_H
