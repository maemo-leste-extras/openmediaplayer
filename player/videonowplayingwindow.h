#ifndef VIDEONOWPLAYINGWINDOW_H
#define VIDEONOWPLAYINGWINDOW_H

#include <gio/gio.h>

#include <QMainWindow>
#include <QIcon>
#include <QTimer>
#include <QtDBus>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>

#include "ui_videonowplayingwindow.h"
#include "includes.h"
#include "confirmdialog.h"
#include "rotator.h"
#include "missioncontrol.h"
#include "radionowplayingwindow.h"
#include "metadatadialog.h"
#include "bookmarkdialog.h"

#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
// we need the udef, otherwise moc_videonowplayingwindow.cpp will fail to
// compile
#undef Bool
#include <QMaemo5InformationBox>
#include <QSpacerItem>
#include "maemo5deviceevents.h"
#include "sharedialog.h"

#include "mafw/mafwregistryadapter.h"
#include "mafw/mafwplaylistmanageradapter.h"

namespace Ui {
    class VideoNowPlayingWindow;
}

class VideoNowPlayingWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit VideoNowPlayingWindow(QWidget *parent = 0, MafwRegistryAdapter *mafwRegistry = 0, bool overlay = false);
    ~VideoNowPlayingWindow();
    bool eventFilter(QObject *object, QEvent *event);

    void play();

protected:
    void mouseReleaseEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void changeEvent(QEvent *e);
    void resizeEvent(QResizeEvent *e);
    void paintEvent(QPaintEvent *e);
private:
    Ui::VideoNowPlayingWindow *ui;
    void setIcons();
    void connectSignals();
    void startPositionTimer();
    void updateDNDAtom();
    void showOverlay(bool show);
    void switchToRadio();
    static QString errorMessage(int code, const QString &message);

    QTimer *volumeTimer;
    QTimer *positionTimer;
    QTimer *keyRepeatTimer;

    QString currentObjectId;
    QString playedObjectId;

    QString uri;
    Rotator::Orientation rotatorPolicy;
    bool lazySliders;
    bool reverseTime;
    bool showSettings;
    bool fitToScreen;
    bool portrait;
    bool overlayVisible;
    bool overlayRequestedByUser;
    bool playWhenReady;
    bool saveStateOnClose;
    bool gotInitialStopState;
    bool gotInitialPlayState;
    bool gotCurrentPlayState;
    bool isMediaSeekable;
    bool buttonWasDown;
    bool resized;
    bool closeClicked;
    int keyToRepeat;
    MafwRegistryAdapter *mafwRegistry;
    MafwRendererAdapter *mafwRenderer;
    MafwSourceAdapter *mafwSource;
    int colorkey;
    int mafwState;
    int videoLength;
    int resumePosition;
    int currentPosition;
    int videoWidth;
    int videoHeight;

private slots:
    void setFitToScreen(bool enable);
    void setContinuousPlayback(bool enable);
    void toggleOverlay();
    void toggleSettings();
    void toggleVolumeSlider();
    void onDetailsClicked();
    void onBookmarkClicked();
    void onShareClicked();
    void onDeleteClicked();
    void onVolumeSliderPressed();
    void onVolumeSliderReleased();
    void onPrevButtonClicked();
    void onNextButtonClicked();
    void onMetadataChanged(QString key, QVariant value);
    void onScreenLocked(bool locked);
    void onCloseClicked();
    void repeatKey();
    void togglePlayback();
    void slowFwd();
    void slowRev();
    void fastFwd();
    void fastRev();
    void onMediaChanged(int, QString objectId);
    void onPropertyChanged(const QString &name, const QVariant &value);
    void onStateChanged(MafwPlayState state);
    void onStatusReceived(MafwPlaylist *, uint index, MafwPlayState, QString objectId);
    void onBufferingInfo(float status);
    void onPositionChanged(int position);
    void onErrorOccurred(uint domain, int code, const QString &message);
    void onPositionSliderPressed();
    void onPositionSliderReleased();
    void onPositionSliderMoved(int);
};

#endif // VIDEONOWPLAYINGWINDOW_H
