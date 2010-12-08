#ifndef NOWPLAYINGWINDOW_H
#define NOWPLAYINGWINDOW_H

#include <QMainWindow>
#ifdef Q_WS_MAEMO_5
#include <libosso.h>
#endif

namespace Ui {
    class NowPlayingWindow;
}

class NowPlayingWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit NowPlayingWindow(QWidget *parent = 0);
    ~NowPlayingWindow();

private:
    Ui::NowPlayingWindow *ui;
#ifdef Q_WS_MAEMO_5
    osso_context_t *osso_context;
#endif
    void setButtonIcons();

private slots:
    void toggleVolumeSlider();
    void showFMTXDialog();
};

#endif // NOWPLAYINGWINDOW_H
