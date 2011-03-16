#ifndef VIDEONOWPLAYINGWINDOW_H
#define VIDEONOWPLAYINGWINDOW_H

#include <QMainWindow>
#include <QIcon>
#include <QTimer>
#include <QtDBus>
#include <QDesktopWidget>

#include "ui_videonowplayingwindow.h"
#include "includes.h"

#ifdef Q_WS_MAEMO_5
    #include "qmaemo5rotator.h"
#endif

namespace Ui {
    class VideoNowPlayingWindow;
}

class VideoNowPlayingWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit VideoNowPlayingWindow(QWidget *parent = 0);
    ~VideoNowPlayingWindow();

private:
    Ui::VideoNowPlayingWindow *ui;
    void setIcons();
    void connectSignals();
    QTimer *volumeTimer;
    bool portrait;
#ifdef Q_WS_MAEMO_5
    QMaemo5Rotator *rotator;
    void setDNDAtom(bool dnd);
#endif

private slots:
    void toggleVolumeSlider();
    void volumeWatcher();
    void orientationChanged();
#ifdef MAFW
    void onVolumeChanged(const QDBusMessage &msg);
#endif
#ifdef Q_WS_MAEMO_5
    void onPortraitMode();
    void onLandscapeMode();
#endif
};

#endif // VIDEONOWPLAYINGWINDOW_H
