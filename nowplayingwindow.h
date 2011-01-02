#ifndef NOWPLAYINGWINDOW_H
#define NOWPLAYINGWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QtDBus>
#include <QTimer>
#include <mirror.h>
#include <cqgraphicsview.h>
#ifdef Q_WS_MAEMO_5
#include <QLibrary>
#include <libosso.h>
#include "mafwrendereradapter.h"
#include "fmtxdialog.h"
#endif

#include "ui_nowplayingwindow.h"

#define prevButtonIcon "/etc/hildon/theme/mediaplayer/Back.png"
#define playButtonIcon "/etc/hildon/theme/mediaplayer/Play.png"
#define pauseButtonIcon "/etc/hildon/theme/mediaplayer/Pause.png"
#define nextButtonIcon "/etc/hildon/theme/mediaplayer/Forward.png"
#define repeatButtonIcon "/etc/hildon/theme/mediaplayer/Repeat.png"
#define repeatButtonPressedIcon "/etc/hildon/theme/mediaplayer/RepeatPressed.png"
#define shuffleButtonIcon "/etc/hildon/theme/mediaplayer/Shuffle.png"
#define shuffleButtonPressed "/etc/hildon/theme/mediaplayer/SufflePressed.png"
#define volumeButtonIcon "/usr/share/icons/hicolor/64x64/hildon/mediaplayer_volume.png"
#define albumImage "/usr/share/icons/hicolor/295x295/hildon/mediaplayer_default_album.png"

namespace Ui {
    class NowPlayingWindow;
}

enum npSongUserRoles { npUserRoleName=Qt::UserRole, npUserRoleSongName };

class PlayListDelegate : public QStyledItemDelegate
{
public:
        explicit PlayListDelegate(QObject *parent=0) : QStyledItemDelegate(parent) {}
        virtual ~PlayListDelegate() {}

        void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

class MafwRendererAdapter;


class NowPlayingWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit NowPlayingWindow(QWidget *parent = 0, MafwRendererAdapter* mra = 0);
    ~NowPlayingWindow();
    void listSongs(QString);

public slots:
    void onMetadataChanged(int, int, QString, QString, QString);

private:
    Ui::NowPlayingWindow *ui;
#ifdef Q_WS_MAEMO_5
    FMTXDialog *fmtxDialog;
    osso_context_t *osso_context;
    MafwRendererAdapter* mafwrenderer;
#endif
    void setButtonIcons();
    void connectSignals();
    QTimer *volumeTimer;
    QGraphicsScene *albumArtScene;
    mirror *m;

private slots:
    void toggleVolumeSlider();
    void showFMTXDialog();
    void orientationChanged();
    void toggleList();
#ifdef Q_WS_MAEMO_5
    void onVolumeChanged(const QDBusMessage &msg);
#endif
    void stateChanged(int state);
    void metadataChanged(QString name, QVariant value);
    void volumeWatcher();
    void setAlbumImage(QString);
};

#endif // NOWPLAYINGWINDOW_H
