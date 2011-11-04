/**************************************************************************
    This file is part of Open MediaPlayer
    Copyright (C) 2010-2011 Mohammad Abu-Garbeyyeh

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "videonowplayingwindow.h"
#ifdef Q_WS_MAEMO_5
#include <QtGui/QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#endif

VideoNowPlayingWindow::VideoNowPlayingWindow(QWidget *parent, MafwAdapterFactory *factory) :
    QMainWindow(parent),
    ui(new Ui::VideoNowPlayingWindow)
#ifdef MAFW
    ,mafwFactory(factory),
    mafwrenderer(factory->getRenderer()),
    mafwTrackerSource(factory->getTrackerSource()),
    playlist(factory->getPlaylistAdapter())
#endif
{
    /* Make Qt do the work of keeping the overlay the magic color  */
    QWidget::setBackgroundRole(QPalette::Window);
    QWidget::setAutoFillBackground(true);
    QPalette overlayPalette = QWidget::palette();
    overlayPalette.setColor
        (QPalette::Window,
         colorKey());
    QWidget::setPalette(overlayPalette);
    ui->setupUi(this);
    setAttribute(Qt::WA_OpaquePaintEvent);
    ui->widget->setAttribute(Qt::WA_NativeWindow);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    quint32 disable = {0};
    Atom winPortraitModeSupportAtom = XInternAtom(QX11Info::display(), "_HILDON_PORTRAIT_MODE_SUPPORT", false);
    XChangeProperty(QX11Info::display(), winId(), winPortraitModeSupportAtom, XA_CARDINAL, 32, PropModeReplace, (uchar*) &disable, 1);
    this->orientationChanged();
    this->onLandscapeMode();
    //http://www.gossamer-threads.com/lists/maemo/developers/54239
#endif
    setAttribute(Qt::WA_DeleteOnClose);
    volumeTimer = new QTimer(this);
    volumeTimer->setInterval(3000);

    positionTimer = new QTimer(this);
    positionTimer->setInterval(1000);

    lazySliders = QSettings().value("main/lazySliders").toBool();
    reverseTime = QSettings().value("main/reverseTime").toBool();

    this->isOverlayVisible = true;
    this->gotInitialState = false;
#ifdef MAFW
    this->errorOccured = false;
#endif

    this->setIcons();
    this->connectSignals();
    ui->volumeSlider->hide();

    ui->currentPositionLabel->installEventFilter(this);

    this->showOverlay(false);

#ifdef MAFW
    mafwrenderer->setColorKey(199939);
    mafwrenderer->getStatus();
    mafwrenderer->getVolume();
    ui->toolbarOverlay->setStyleSheet(ui->controlOverlay->styleSheet());
#endif
    this->grabKeyboard();
}

VideoNowPlayingWindow::~VideoNowPlayingWindow()
{
#ifdef MAFW
    if (this->mafwState != Paused && this->mafwState != Stopped)
        mafwrenderer->pause();

    GHashTable* metadata = mafw_metadata_new();
    if (currentPosition == 0)
        mafw_metadata_add_str(metadata, MAFW_METADATA_KEY_PAUSED_THUMBNAIL_URI, "");
    mafw_metadata_add_int(metadata, MAFW_METADATA_KEY_PAUSED_POSITION, currentPosition);
    mafwTrackerSource->setMetadata(objectIdToPlay.toUtf8(), metadata);
    mafw_metadata_release(metadata);
#endif
    delete ui;
}

bool VideoNowPlayingWindow::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease)
        return true;

    if (event->type() == QEvent::MouseButtonPress) {
        reverseTime = !reverseTime;
        QSettings().setValue("main/reverseTime", reverseTime);
        ui->currentPositionLabel->setText(time_mmss(reverseTime ? ui->progressBar->value()-videoLength :
                                                                  ui->progressBar->value()));
        return true;
    }
    return false;
}

void VideoNowPlayingWindow::setIcons()
{
    ui->wmCloseButton->setIcon(QIcon(wmCloseIcon));
    ui->prevButton->setIcon(QIcon(prevButtonIcon));
    ui->playButton->setIcon(QIcon(playButtonIcon));
    ui->nextButton->setIcon(QIcon(nextButtonIcon));
    ui->deleteButton->setIcon(QIcon::fromTheme(deleteButtonIcon));
    ui->shareButton->setIcon(QIcon::fromTheme(shareButtonIcon));
    ui->volumeButton->setIcon(QIcon(volumeButtonIcon));
}

void VideoNowPlayingWindow::connectSignals()
{
    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(toggleVolumeSlider()));
    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(volumeWatcher()));
    connect(volumeTimer, SIGNAL(timeout()), this, SLOT(toggleVolumeSlider()));
    connect(ui->volumeSlider, SIGNAL(sliderPressed()), this, SLOT(onVolumeSliderPressed()));
    connect(ui->volumeSlider, SIGNAL(sliderReleased()), this, SLOT(onVolumeSliderReleased()));
    connect(ui->shareButton, SIGNAL(clicked()), this, SLOT(onShareClicked()));
    connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(onDeleteClicked()));
#ifdef MAFW
    connect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));
    connect(mafwrenderer, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));
    connect(mafwrenderer, SIGNAL(signalGetPosition(int,QString)), this, SLOT(onPositionChanged(int,QString)));
    connect(mafwrenderer, SIGNAL(mediaIsSeekable(bool)), ui->progressBar, SLOT(setEnabled(bool)));
    connect(positionTimer, SIGNAL(timeout()), mafwrenderer, SLOT(getPosition()));
    connect(mafwTrackerSource, SIGNAL(signalMetadataResult(QString,GHashTable*,QString)),
            this, SLOT(onSourceMetadataRequested(QString,GHashTable*,QString)));
    connect(mafwrenderer, SIGNAL(signalGetVolume(int)), ui->volumeSlider, SLOT(setValue(int)));
    connect(ui->volumeSlider, SIGNAL(sliderMoved(int)), mafwrenderer, SLOT(setVolume(int)));
    connect(ui->progressBar, SIGNAL(sliderPressed()), this, SLOT(onSliderPressed()));
    connect(ui->progressBar, SIGNAL(sliderReleased()), this, SLOT(onSliderReleased()));
    connect(ui->progressBar, SIGNAL(sliderMoved(int)), this, SLOT(onSliderMoved(int)));
    QDBusConnection::sessionBus().connect("com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer",
                                          "/com/nokia/mafw/renderer/gstrenderer",
                                          "com.nokia.mafw.extension",
                                          "property_changed",
                                          this, SLOT(onPropertyChanged(const QDBusMessage &)));

    QDBusConnection::sessionBus().connect("",
                                          "/com/nokia/mafw/renderer/gstrenderer",
                                          "com.nokia.mafw.extension",
                                          "error",
                                          this, SLOT(onErrorOccured(const QDBusMessage &)));

#endif
}

void VideoNowPlayingWindow::onDeleteClicked()
{
#ifdef MAFW
    mafwrenderer->pause();

    QMessageBox confirmDelete(QMessageBox::NoIcon,
                                  tr("Delete video?"),
                                  tr("Are you sure you want to delete this video?"),
                                  QMessageBox::Yes | QMessageBox::No,
                                  this);
    confirmDelete.exec();

    if (confirmDelete.result() == QMessageBox::Yes) {
        mafwTrackerSource->destroyObject(objectIdToPlay.toUtf8());
        this->close();
    }
#endif
}

void VideoNowPlayingWindow::onShareClicked()
{
#ifdef MAFW
    mafwrenderer->pause();
    mafwTrackerSource->getUri(this->objectIdToPlay.toUtf8());
    connect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));
#endif
}

#ifdef MAFW
void VideoNowPlayingWindow::onShareUriReceived(QString objectId, QString uri)
{
    disconnect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));

    if (objectId != this->objectIdToPlay)
        return;

    QStringList list;
    QString clip;
    clip = uri;
#ifdef DEBUG
    qDebug() << "Sending file:" << clip;
#endif
    list.append(clip);
#ifdef Q_WS_MAEMO_5
    Share *share = new Share(this, list);
    share->setAttribute(Qt::WA_DeleteOnClose);
    share->show();
#endif
}
#endif

void VideoNowPlayingWindow::onVolumeSliderPressed()
{
    volumeTimer->stop();
#ifdef MAFW
    mafwrenderer->setVolume(ui->volumeSlider->value());
#endif
}

void VideoNowPlayingWindow::onVolumeSliderReleased()
{
    volumeTimer->start();
#ifdef MAFW
    mafwrenderer->setVolume(ui->volumeSlider->value());
#endif
}


void VideoNowPlayingWindow::toggleVolumeSlider()
{
    if (ui->volumeSlider->isHidden()) {
        ui->buttonWidget->hide();
        ui->volumeSlider->show();
    } else {
        ui->volumeSlider->hide();
        ui->buttonWidget->show();
        if(volumeTimer->isActive())
            volumeTimer->stop();
    }
}

#ifdef MAFW
void VideoNowPlayingWindow::onPropertyChanged(const QDBusMessage &msg)
{
    /*dbus-send --print-reply --type=method_call --dest=com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer \
                 /com/nokia/mafw/renderer/gstrenderer com.nokia.mafw.extension.get_extension_property string:volume*/
    if (msg.arguments()[0].toString() == "volume") {
        int volumeLevel = qdbus_cast<QVariant>(msg.arguments()[1]).toInt();
#ifdef DEBUG
        qDebug() << QString::number(volumeLevel);
#endif
        ui->volumeSlider->setValue(volumeLevel);
    }

    else if (msg.arguments()[0].toString() == "colorkey") {
        colorkey = qdbus_cast<QVariant>(msg.arguments()[1]).toInt();
    }
}
#endif

void VideoNowPlayingWindow::volumeWatcher()
{
    if(!ui->volumeSlider->isHidden())
        volumeTimer->start();
}

#ifdef MAFW
void VideoNowPlayingWindow::stateChanged(int state)
{
    this->mafwState = state;

    if (state == Paused) {
        ui->playButton->setIcon(QIcon(playButtonIcon));
        disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
        connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(resume()));
#ifdef Q_WS_MAEMO_5
        this->setDNDAtom(false);
#endif
        mafwrenderer->getPosition();
        this->pausedPosition = this->currentPosition;
        mafwrenderer->getPosition();
        if (positionTimer->isActive())
            positionTimer->stop();
    }
    else if (state == Playing) {
        ui->playButton->setIcon(QIcon(pauseButtonIcon));
        disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
        connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(pause()));
        if (pausedPosition != -1 && pausedPosition != 0)
            mafwrenderer->setPosition(SeekAbsolute, pausedPosition);
        mafwrenderer->getPosition();
#ifdef Q_WS_MAEMO_5
        this->setDNDAtom(true);
#endif
        if (!positionTimer->isActive())
            positionTimer->start();
    }
    else if (state == Stopped) {
        currentPosition = 0;
        ui->playButton->setIcon(QIcon(playButtonIcon));
        disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
        connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(play()));
#ifdef Q_WS_MAEMO_5
        this->setDNDAtom(false);
#endif
        if (positionTimer->isActive())
            positionTimer->stop();
        if (this->gotInitialState && !this->errorOccured) {
            this->close();
            delete this; // why is it not deleted automatically, despite WA_DeleteOnClose?
        }
    }
    else if (state == Transitioning) {
        ui->progressBar->setEnabled(false);
        ui->progressBar->setValue(0);
        ui->currentPositionLabel->setText("00:00");
    }
}
#endif

void VideoNowPlayingWindow::orientationChanged()
{
    //QRect screenGeometry = QApplication::desktop()->screenGeometry();
    QRect screenGeometry = QRect(0,0,800,480);
    ui->controlOverlay->setGeometry((screenGeometry.width() / 2)-(ui->controlOverlay->width()/2),
                                    (screenGeometry.height() / 2)-(ui->controlOverlay->height()/2),
                                    ui->controlOverlay->width(), ui->controlOverlay->height());
    ui->toolbarOverlay->setGeometry(0, screenGeometry.height()-ui->toolbarOverlay->height(),
                                    screenGeometry.width(), ui->toolbarOverlay->height());
    ui->wmCloseButton->setGeometry(screenGeometry.width()-ui->wmCloseButton->width(), 0,
                                   ui->wmCloseButton->width(), ui->wmCloseButton->height());
    ui->widget->setGeometry(0, 0, screenGeometry.width(), screenGeometry.height());
}

#ifdef Q_WS_MAEMO_5
void VideoNowPlayingWindow::onLandscapeMode()
{
    this->setIcons();
    ui->deleteButton->show();
    ui->shareButton->show();
    ui->volumeButton->show();
    //QRect screenGeometry = QApplication::desktop()->screenGeometry();
    QRect screenGeometry = QRect(0,0,800,480);
    ui->wmCloseButton->setIconSize(QSize(112, 56));
    ui->wmCloseButton->setGeometry(screenGeometry.width()-112, 0, 112, 56);
    ui->controlLayout->setDirection(QBoxLayout::LeftToRight);
    ui->controlOverlay->setGeometry(230, 170, 318, 114);
    if(ui->toolbarOverlay->isHidden())
        ui->toolbarOverlay->show();
}

void VideoNowPlayingWindow::setDNDAtom(bool dnd)
{
    quint32 enable;
    if (dnd)
        enable = 1;
    else
        enable = 0;
    Atom winDNDAtom = XInternAtom(QX11Info::display(), "_HILDON_DO_NOT_DISTURB", false);
    XChangeProperty(QX11Info::display(), winId(), winDNDAtom, XA_INTEGER, 32, PropModeReplace, (uchar*) &enable, 1);
}
#endif

void VideoNowPlayingWindow::playObject(QString objectId)
{
#ifdef MAFW
    this->objectIdToPlay = objectId;
    this->mafwTrackerSource->getMetadata(objectId.toUtf8(), MAFW_SOURCE_LIST(MAFW_METADATA_KEY_DURATION,
                                                                             MAFW_METADATA_KEY_PAUSED_POSITION));
    QTimer::singleShot(200, this, SLOT(playVideo()));
#endif
}

#ifdef MAFW
void VideoNowPlayingWindow::onSourceMetadataRequested(QString, GHashTable *metadata, QString error)
{
    if (metadata != NULL) {
        int duration;
        GValue *v;

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_DURATION);
        duration = v ? g_value_get_int (v) : Duration::Unknown;

        this->videoLength = duration;

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_PAUSED_POSITION);
        pausedPosition = v ? g_value_get_int (v) : -1;
#ifdef MAFW
        if (pausedPosition != -1)
            qDebug() << pausedPosition;
#endif

        ui->videoLengthLabel->setText(time_mmss(duration));
        ui->progressBar->setRange(0, duration);
    }

    if(!error.isNull() && !error.isEmpty())
        qDebug() << error;
}

void VideoNowPlayingWindow::playVideo()
{
    unsigned int windowId = ui->widget->winId();
    QApplication::syncX();
    mafwrenderer->setWindowXid(windowId);

    mafwrenderer->playObject(this->objectIdToPlay.toUtf8());
}
#endif

void VideoNowPlayingWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QBrush brush;
    brush.setColor(QColor(3, 13, 3));
    painter.setBrush(brush);
    QPalette palette = this->palette();
    palette.setColor(this->backgroundRole(), QColor(3, 13, 3));
}

void VideoNowPlayingWindow::mouseReleaseEvent(QMouseEvent *)
{
    this->showOverlay(!this->isOverlayVisible);
}

void VideoNowPlayingWindow::keyPressEvent(QKeyEvent *event)
{
#ifdef MAFW
    if (event->key() == Qt::Key_Space) {
        if (this->mafwState == Playing)
            mafwrenderer->pause();
        else if (this->mafwState == Paused)
            mafwrenderer->resume();
        else if (this->mafwState == Stopped)
            mafwrenderer->play();
    }
    else if (event->key() == Qt::Key_Left) {
        mafwrenderer->setPosition(SeekRelative, -10);
        if (mafwState == Paused)
            mafwrenderer->getPosition();
    }
    else if (event->key() == Qt::Key_Right) {
        mafwrenderer->setPosition(SeekRelative, 10);
        if (mafwState == Paused)
            mafwrenderer->getPosition();
    }
    else if (event->key() == Qt::Key_Up) {
        mafwrenderer->setPosition(SeekRelative, 60);
        if (mafwState == Paused)
            mafwrenderer->getPosition();
    }
    else if (event->key() == Qt::Key_Down) {
        mafwrenderer->setPosition(SeekRelative, -60);
        if (mafwState == Paused)
            mafwrenderer->getPosition();
    }
#endif
}

void VideoNowPlayingWindow::keyReleaseEvent(QKeyEvent *event)
{
#ifdef MAFW
#endif
}

void VideoNowPlayingWindow::showOverlay(bool show)
{
    ui->controlOverlay->setHidden(!show);
    ui->toolbarOverlay->setHidden(!show);
    ui->wmCloseButton->setHidden(!show);

    this->isOverlayVisible = show;

#ifdef MAFW
    if (!show && this->positionTimer->isActive())
        this->positionTimer->stop();
    else if (show && !this->positionTimer->isActive() && this->mafwState != Stopped) {
        this->positionTimer->start();
        mafwrenderer->getPosition();
    }
#endif
}

void VideoNowPlayingWindow::onSliderPressed()
{
    this->onSliderMoved(ui->progressBar->value());
}

void VideoNowPlayingWindow::onSliderReleased()
{
#ifdef MAFW
    mafwrenderer->setPosition(SeekAbsolute, ui->progressBar->value());
    ui->currentPositionLabel->setText(time_mmss(reverseTime ? ui->progressBar->value()-videoLength :
                                                              ui->progressBar->value()));
#endif
}

void VideoNowPlayingWindow::onSliderMoved(int position)
{
    ui->currentPositionLabel->setText(time_mmss(reverseTime ? position-videoLength : position));
#ifdef MAFW
    if (!lazySliders)
        mafwrenderer->setPosition(SeekAbsolute, position);
#endif
}

#ifdef MAFW
void VideoNowPlayingWindow::onGetStatus(MafwPlaylist*, uint, MafwPlayState state, const char *, QString)
{
    this->stateChanged(state);
    this->gotInitialState = true;
}

void VideoNowPlayingWindow::onPositionChanged(int position, QString)
{
    this->currentPosition = position;
    if (this->mafwState == Paused)
         this->pausedPosition = this->currentPosition;
    if (!ui->progressBar->isSliderDown() && ui->progressBar->isVisible()) {
        ui->currentPositionLabel->setText(time_mmss(reverseTime ? position-videoLength : position));
        if (ui->progressBar->isEnabled())
            ui->progressBar->setValue(position);
    }
}
#endif

#ifdef MAFW
void VideoNowPlayingWindow::onErrorOccured(const QDBusMessage &msg)
{
    this->errorOccured = true;
    QString errorMsg;

    if (msg.arguments()[0] == "com.nokia.mafw.error.renderer") {
        errorMsg.append(tr("Unable to play media"));
        errorMsg.append("\n");

        if (msg.arguments()[1] == MAFW_RENDERER_ERROR_NO_MEDIA)
            errorMsg.append(tr("Media not found"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_URI_NOT_AVAILABLE)
            errorMsg.append(tr("URI not available"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_INVALID_URI)
            errorMsg.append(tr("Invalid URI"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_MEDIA_NOT_FOUND)
            errorMsg.append(tr("Unable to open media"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_STREAM_DISCONNECTED)
            errorMsg.append(tr("Playback stream no longer available"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_TYPE_NOT_AVAILABLE)
            errorMsg.append(tr("Could not determine MIME-type"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_PLAYBACK)
            errorMsg.append(tr("General error occured, unable to continue playback"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_UNABLE_TO_PERFORM)
            errorMsg.append(tr("General error occured"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_UNSUPPORTED_TYPE)
            errorMsg.append(tr("Unsupported media"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_UNSUPPORTED_RESOLUTION)
            errorMsg.append(tr("Unsupported resolution"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_UNSUPPORTED_FPS)
            errorMsg.append(tr("Unsupported framerate"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_DRM)
            errorMsg.append(tr("Media is protected by DRM"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_DEVICE_UNAVAILABLE)
            errorMsg.append(tr("System sound device is unavailable"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_CORRUPTED_FILE)
            errorMsg.append(tr("Media corrupted"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_PLAYLIST_PARSING) {
            errorMsg.append(tr("Error while parsing playlist"));
            errorMsg.append(tr("Playlist may be corrupt or empty"));
        }
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_CODEC_NOT_FOUND) {
            errorMsg.append(tr("Codec not found:") + "\n");
            errorMsg.append(msg.arguments()[2].toString());
        }
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_VIDEO_CODEC_NOT_FOUND) {
            errorMsg.append(tr("Video codec not found:") + "\n");
            errorMsg.append(msg.arguments()[2].toString());
        }
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_AUDIO_CODEC_NOT_FOUND) {
            errorMsg.append(tr("Audio codec not found:") + "\n");
            errorMsg.append(msg.arguments()[2].toString());
        }
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_NO_PLAYLIST)
            errorMsg.append(tr("No playlist assigned"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_INDEX_OUT_OF_BOUNDS)
            errorMsg.append(tr("Media index is not in bound with playlist items"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_CANNOT_PLAY)
            errorMsg.append(tr("Unable to start playback"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_CANNOT_STOP)
            errorMsg.append(tr("Unable to stop playback"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_CANNOT_PAUSE)
            errorMsg.append(tr("Unable to pause playback"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_CANNOT_SET_POSITION)
            errorMsg.append(tr("Unable to seek position in media"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_CANNOT_GET_POSITION)
            errorMsg.append(tr("Unable to retrieve current position in media"));
        else if (msg.arguments()[1] == MAFW_RENDERER_ERROR_CANNOT_GET_STATUS)
            errorMsg.append(tr("Unable to get current playback status"));

#ifdef Q_WS_MAEMO_5
        QMaemo5InformationBox *box = new QMaemo5InformationBox(this);
        box->setAttribute(Qt::WA_DeleteOnClose);

        QWidget *widget = new QWidget(box);
        QSpacerItem *spacer = new QSpacerItem(90, 20, QSizePolicy::Fixed, QSizePolicy::Maximum);

        QLabel *errorLabel = new QLabel(box);

        QHBoxLayout *layout = new QHBoxLayout(widget);

        layout->addItem(spacer);
        layout->addWidget(errorLabel);
        layout->setSpacing(0);

        widget->setLayout(layout);

        // Bad padding in default widget, use tabbing to cover it up, sigh :/
        errorLabel->setText("\n" + errorMsg + "\n");
        errorLabel->setAlignment(Qt::AlignLeft);
        errorLabel->setWordWrap(true);

        box->setWidget(widget);
        box->setTimeout(QMaemo5InformationBox::NoTimeout);

        connect(box, SIGNAL(clicked()), this, SLOT(close()));

        box->exec();
#else
        QMessageBox::critical(this, tr("Unable to play media") + "\n", errorMsg);
#endif
    }
}
#endif

