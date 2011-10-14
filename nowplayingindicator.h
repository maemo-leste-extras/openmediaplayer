#ifndef NOWPLAYINGINDICATOR_H
#define NOWPLAYINGINDICATOR_H

#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QList>
#include <QTimer>
#include <QtDBus>
#include <QDesktopWidget>
#include <QMouseEvent>
#ifdef Q_WS_MAEMO_5
    #include "mafw/mafwadapterfactory.h"
    #include "maemo5deviceevents.h"
#endif
#include "ui_nowplayingindicator.h"
#include "includes.h"
#include "nowplayingwindow.h"
#include "radionowplayingwindow.h"
#include "videonowplayingwindow.h"

namespace Ui {
    class NowPlayingIndicator;
}

class MafwRendererAdapter;

class NowPlayingIndicator : public QWidget
{
    Q_OBJECT

public:
    explicit NowPlayingIndicator(QWidget *parent = 0);
    ~NowPlayingIndicator();
    void triggerAnimation();
#ifdef MAFW
    QString currentObjectId();
    void setFactory(MafwAdapterFactory *mafwFactory = 0);
#endif

public slots:
    void stopAnimation();
    void autoSetVisibility();
    void inhibit();
    void restore();
    void poke();

signals:
    void clicked();

private:
    Ui::NowPlayingIndicator *ui;
    void paintEvent(QPaintEvent*);
    void mouseReleaseEvent(QMouseEvent *);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void connectSignals();
    QList<QPixmap> images;
    QTimer *timer;
    QTimer *pokeTimer;
    int mafwState;
    QString indicatorImage;
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter *mafwrenderer;
    MafwPlaylistAdapter *playlist;
    QMainWindow *window;
    QString rendererObjectId;
    bool ready;
    bool poked;
    int inhibited;
#endif
#ifdef Q_WS_MAEMO_5
    Maemo5DeviceEvents *deviceEvents;
#endif
    int frame;

private slots:
#ifdef Q_WS_MAEMO_5
    void onTkLockChanged(bool);
#endif
#ifdef MAFW
    void onStateChanged(int);
    void onMediaChanged(int, char* objectId);
    void onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString);
    void onPlaylistReady();
#endif
    void startAnimation();
    void onWindowDestroyed();
    void onNowPlayingWindowHidden();
    void onPokeTimeout();
};

#endif // NOWPLAYINGINDICATOR_H
