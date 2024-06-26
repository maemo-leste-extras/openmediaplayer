#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "basewindow.h"

#include <QMainWindow>
#include <QPainter>
#include <QDebug>
#include <QtGui>
#include <QDBusConnection>

#include "missioncontrol.h"
#include "musicwindow.h"
#include "videoswindow.h"
#include "internetradiowindow.h"
#include "ui_mainwindow.h"
#include "nowplayingindicator.h"
#include "includes.h"
#include "settingsdialog.h"
#include "sleeperdialog.h"
#include "pluginscontrol.h"
#include "aboutwindow.h"
#include "delegates/maindelegate.h"
#include "rotator.h"
#include "opendialog.h"
#include "currentplaylistmanager.h"

#include <QMaemo5InformationBox>
#include <QX11Info>
#include <X11/Xlib.h>
// Without this undef, moc_mainwindow.cpp will fail to compile
#undef Bool

#define DBUS_SERVICE   "com.nokia.mediaplayer"
#define DBUS_PATH      "/com/nokia/osso/mediaplayer"
#define DBUS_INTERFACE "com.nokia.mediaplayer"

#include "mafw/mafwregistryadapter.h"
#define TAGSOURCE_AUDIO_PATH     "localtagfs::music/songs"
#define TAGSOURCE_PLAYLISTS_PATH "localtagfs::music/playlists"
#define TAGSOURCE_VIDEO_PATH     "localtagfs::videos"
#define RADIOSOURCE_PATH         "iradiosource::"

namespace Ui {
    class MainWindow;
}

class MainWindow : public BaseWindow
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.nokia.mediaplayer")

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    Q_SCRIPTABLE void open_mp_main_view();
    Q_SCRIPTABLE void open_mp_now_playing();
    Q_SCRIPTABLE void open_mp_now_playing_playback_on();
    Q_SCRIPTABLE void open_mp_radio_playing();
    Q_SCRIPTABLE void open_mp_radio_playing_playback_on();
    Q_SCRIPTABLE void open_mp_car_view();
    Q_SCRIPTABLE void mime_open(const QString &uri);
    Q_SCRIPTABLE void play_automatic_playlist(const QString &playlistName, bool shuffle = false);
    Q_SCRIPTABLE void play_saved_playlist(const QString &playlistName, bool shuffle = false);
    Q_SCRIPTABLE void top_application();

private:
    Ui::MainWindow *ui;
    MusicWindow *musicWindow;
    void paintEvent(QPaintEvent*);
    void loadThemeIcons();
    void setButtonIcons();
    void connectSignals();
    void closeChildren();
    QMaemo5InformationBox *updatingInfoBox;
    QProgressBar *updatingProgressBar;
    QLabel *updatingLabel;
    MafwRegistryAdapter *mafwRegistry;
    MafwRendererAdapter *mafwRenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwSourceAdapter *mafwRadioSource;
    CurrentPlaylistAdapter *playlist;
    uint browsePlaylistId;
    uint playlistToken;
    void countSongs();
    void countVideos();
    void openDirectory(const QString &uri, const QString &objectIdToPlay, Media::Type type);
    void convertObjectId(QString &objectId, const char *basePath);

private slots:
    void onOrientationChanged(int w, int h);
    void showAbout();
    void processListClicks(QListWidgetItem*);
    void openSettings();
    void openSleeperDialog();
    void reloadSettings();
    void showMusicWindow();
    void showVideosWindow();
    void showInternetRadioWindow();
    void onShuffleAllClicked();
    NowPlayingWindow* createNowPlayingWindow();
    void createVideoNowPlayingWindow();
    void onChildOpened();
    void onNowPlayingWindowHidden();
    void onChildClosed();
    void setupPlayback();
    void countAudioVideoResult(QString objectId, GHashTable *metadata);
    void countRadioResult(QString objectId, GHashTable *metadata);
    void countRadioStations();
    void onAddFinished(uint token);
    void onSourceUpdating(int progress, int processed_items, int remaining_items, int remaining_time);
    void onContainerChanged(QString objectId);
    void registerDbusService();
    void onScreenLocked(bool locked);
    void takeScreenshot();
};

#endif // MAINWINDOW_H
