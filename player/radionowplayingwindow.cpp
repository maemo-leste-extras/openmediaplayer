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

#include "radionowplayingwindow.h"

RadioNowPlayingWindow::RadioNowPlayingWindow(QWidget *parent, MafwRegistryAdapter *mafwRegistry) :
    BaseWindow(parent),
    ui(new Ui::RadioNowPlayingWindow),
    mafwRegistry(mafwRegistry),
    mafwRenderer(mafwRegistry->renderer()),
    playlist(mafwRegistry->playlist())
{
    ui->setupUi(this);
    menuBar()->hide();

    setAttribute(Qt::WA_DeleteOnClose);

    QPalette palette;
    QColor secondaryColor = QMaemo5Style::standardColor("SecondaryTextColor");
    palette.setColor(QPalette::WindowText, secondaryColor);

    ui->songLabel->setPalette(palette);

    albumArtScene = new QGraphicsScene(ui->albumArtView);
    setAlbumImage(defaultRadioImage);

    lazySliders = QSettings().value("main/lazySliders").toBool();

    mafwState = Transitioning;

    volumeTimer = new QTimer(this);
    volumeTimer->setInterval(3000);

    positionTimer = new QTimer(this);
    positionTimer->setInterval(1000);

    this->setIcons();
    this->connectSignals();

    MetadataWatcher *mw = MissionControl::acquire()->metadataWatcher();
    connect(mw, SIGNAL(metadataChanged(QString,QVariant)), this, SLOT(onMetadataChanged(QString,QVariant)));
    QMap<QString,QVariant> metadata = mw->metadata();
    onMetadataChanged(MAFW_METADATA_KEY_TITLE, metadata.value(MAFW_METADATA_KEY_TITLE));
    onMetadataChanged(MAFW_METADATA_KEY_ARTIST, metadata.value(MAFW_METADATA_KEY_ARTIST));
    onMetadataChanged(MAFW_METADATA_KEY_ORGANIZATION, metadata.value(MAFW_METADATA_KEY_ORGANIZATION));
    onMetadataChanged(MAFW_METADATA_KEY_IS_SEEKABLE, metadata.value(MAFW_METADATA_KEY_IS_SEEKABLE));
    onMetadataChanged(MAFW_METADATA_KEY_DURATION, metadata.value(MAFW_METADATA_KEY_DURATION));
    onMetadataChanged(MAFW_METADATA_KEY_RENDERER_ART_URI, metadata.value(MAFW_METADATA_KEY_RENDERER_ART_URI));
    onMetadataChanged(MAFW_METADATA_KEY_URI, metadata.value(MAFW_METADATA_KEY_URI));

    networkSession = new QNetworkSession(QNetworkConfigurationManager().defaultConfiguration(), this);

    Rotator::acquire()->addClient(this);

    mafwRenderer->getStatus();
    mafwRenderer->getPosition();
    mafwRenderer->getVolume();
}

RadioNowPlayingWindow::~RadioNowPlayingWindow()
{
    delete ui;
}

void RadioNowPlayingWindow::connectSignals()
{
    QShortcut *shortcut;

    shortcut = new QShortcut(QKeySequence(Qt::Key_Space), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), this, SLOT(togglePlayback()));
    shortcut = new QShortcut(QKeySequence(Qt::Key_Left), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), mafwRenderer, SLOT(previous()));
    shortcut = new QShortcut(QKeySequence(Qt::Key_Right), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), mafwRenderer, SLOT(next()));

    connect(ui->actionFM_transmitter, SIGNAL(triggered()), this, SLOT(showFMTXDialog()));
    connect(ui->actionAdd_radio_bookmark, SIGNAL(triggered()), this, SLOT(showBookmarkDialog()));
    connect(ui->actionDetails, SIGNAL(triggered()), this, SLOT(showDetails()));

    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(toggleVolumeSlider()));
    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(volumeWatcher()));
    connect(volumeTimer, SIGNAL(timeout()), this, SLOT(toggleVolumeSlider()));
    connect(ui->volumeSlider, SIGNAL(sliderPressed()), this, SLOT(onVolumeSliderPressed()));
    connect(ui->volumeSlider, SIGNAL(sliderReleased()), this, SLOT(onVolumeSliderReleased()));

    connect(ui->nextButton, SIGNAL(pressed()), this, SLOT(onNextButtonPressed()));
    connect(ui->nextButton, SIGNAL(released()), this, SLOT(onNextButtonPressed()));
    connect(ui->prevButton, SIGNAL(pressed()), this, SLOT(onPrevButtonPressed()));
    connect(ui->prevButton, SIGNAL(released()), this, SLOT(onPrevButtonPressed()));
    connect(ui->playButton, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onPlayMenuRequested(QPoint)));
    connect(ui->nextButton, SIGNAL(clicked()), this, SLOT(onNextButtonClicked()));
    connect(ui->prevButton, SIGNAL(clicked()), this, SLOT(onPreviousButtonClicked()));

    connect(ui->volumeSlider, SIGNAL(sliderMoved(int)), mafwRenderer, SLOT(setVolume(int)));
    connect(ui->positionSlider, SIGNAL(sliderPressed()), this, SLOT(onPositionSliderPressed()));
    connect(ui->positionSlider, SIGNAL(sliderReleased()), this, SLOT(onPositionSliderReleased()));
    connect(ui->positionSlider, SIGNAL(sliderMoved(int)), this, SLOT(onPositionSliderMoved(int)));
    connect(positionTimer, SIGNAL(timeout()), mafwRenderer, SLOT(getPosition()));

    connect(Maemo5DeviceEvents::acquire(), SIGNAL(screenLocked(bool)), this, SLOT(onScreenLocked(bool)));

    connect(mafwRenderer, SIGNAL(propertyChanged(QString,QVariant)), this, SLOT(onPropertyChanged(QString,QVariant)));
    connect(mafwRenderer, SIGNAL(volumeReceived(int,QString)), ui->volumeSlider, SLOT(setValue(int)));
    connect(mafwRenderer, SIGNAL(positionReceived(int,QString)), this, SLOT(onPositionChanged(int)));
    connect(mafwRenderer, SIGNAL(bufferingInfo(float)), this, SLOT(onBufferingInfo(float)));
    connect(mafwRenderer, SIGNAL(statusReceived(MafwPlaylist*,uint,MafwPlayState,QString,QString)),
            this, SLOT(onStatusReceived(MafwPlaylist*,uint,MafwPlayState)));
}

void RadioNowPlayingWindow::onScreenLocked(bool locked)
{
    if (locked) {
        positionTimer->stop();
    } else {
        startPositionTimer();
    }
}

void RadioNowPlayingWindow::onPropertyChanged(const QString &name, const QVariant &value)
{
    if (name == MAFW_PROPERTY_RENDERER_VOLUME && !ui->volumeSlider->isSliderDown())
        ui->volumeSlider->setValue(value.toInt());
}

void RadioNowPlayingWindow::setIcons()
{
    ui->prevButton->setIcon(QIcon(prevButtonIcon));
    ui->playButton->setIcon(QIcon(playButtonIcon));
    ui->nextButton->setIcon(QIcon(nextButtonIcon));
    ui->volumeButton->setIcon(QIcon(volumeButtonIcon));
}

void RadioNowPlayingWindow::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
        case Qt::Key_Backspace:
            this->close();
            break;

        case Qt::Key_S:
            mafwRenderer->stop();
            break;
    }
}

void RadioNowPlayingWindow::startPositionTimer()
{
    if (!positionTimer->isActive()
    &&  mafwState == Playing
    &&  !Maemo5DeviceEvents::acquire()->isScreenLocked())
    {
        mafwRenderer->getPosition();
        positionTimer->start();
    }
}

void RadioNowPlayingWindow::toggleVolumeSlider()
{
    if (ui->volumeSlider->isHidden()) {
        ui->buttonsWidget->hide();
        ui->volumeSlider->show();
    } else {
        ui->volumeSlider->hide();
        ui->buttonsWidget->show();
        if (volumeTimer->isActive())
            volumeTimer->stop();
    }
}

void RadioNowPlayingWindow::volumeWatcher()
{
    if (!ui->volumeSlider->isHidden())
        volumeTimer->start();
}

void RadioNowPlayingWindow::onVolumeSliderPressed()
{
    volumeTimer->stop();
    mafwRenderer->setVolume(ui->volumeSlider->value());
}

void RadioNowPlayingWindow::onVolumeSliderReleased()
{
    volumeTimer->start();
    mafwRenderer->setVolume(ui->volumeSlider->value());
}

void RadioNowPlayingWindow::onStateChanged(MafwPlayState state)
{
    mafwState = state;

    disconnect(ui->playButton, SIGNAL(pressed()), this, SLOT(onStopButtonPressed()));
    disconnect(ui->playButton, SIGNAL(released()), this, SLOT(onStopButtonPressed()));

    if (state == Transitioning) {
        ui->positionSlider->setMaximum(0);

        if (ui->bufferBar->maximum() == 0) {
            ui->seekWidget->hide();
            ui->bufferBar->show();
        }
    } else {
        if (ui->bufferBar->maximum() == 0) {
            ui->bufferBar->hide();
            ui->seekWidget->show();
        }

        if (state == Paused) {
            ui->positionSlider->setEnabled(isMediaSeekable);

            ui->playButton->setIcon(QIcon(playButtonIcon));
            disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
            connect(ui->playButton, SIGNAL(clicked()), mafwRenderer, SLOT(resume()));

            positionTimer->stop();
            mafwRenderer->getPosition();
        }
        else if (state == Playing) {
            ui->positionSlider->setEnabled(isMediaSeekable);

            disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);

            if (isMediaSeekable) {
                ui->playButton->setIcon(QIcon(pauseButtonIcon));
                connect(ui->playButton, SIGNAL(clicked()), mafwRenderer, SLOT(pause()));
            } else {
                ui->playButton->setIcon(QIcon(stopButtonIcon));
                connect(ui->playButton, SIGNAL(clicked()), mafwRenderer, SLOT(stop()));
                connect(ui->playButton, SIGNAL(pressed()), this, SLOT(onStopButtonPressed()));
                connect(ui->playButton, SIGNAL(released()), this, SLOT(onStopButtonPressed()));
            }

            startPositionTimer();
        }
        else if (state == Stopped) {
            ui->playButton->setIcon(QIcon(playButtonIcon));
            disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
            connect(ui->playButton, SIGNAL(clicked()), this, SLOT(play()));

            positionTimer->stop();
        }
    }

    if (state == Transitioning || state == Stopped) {
        ui->positionSlider->setEnabled(false);
        ui->positionSlider->setValue(0);
        ui->currentPositionLabel->setText(mmss_pos(0));
    }

}

void RadioNowPlayingWindow::play()
{
    if (networkSession->isOpen()) {
        mafwRenderer->play();
    } else {
        connect(networkSession, SIGNAL(opened()), mafwRenderer, SLOT(play()), Qt::UniqueConnection);
        networkSession->open();
    }
}

void RadioNowPlayingWindow::onMediaChanged()
{
    onBufferingInfo(1.0);
}

void RadioNowPlayingWindow::onMetadataChanged(QString key, QVariant value)
{
    if (key == MAFW_METADATA_KEY_TITLE) {
        title = value.toString();
        updateSongLabel();
    }
    else if (key == MAFW_METADATA_KEY_ARTIST) {
        artist = value.toString();
        updateSongLabel();
    }
    else if (key == MAFW_METADATA_KEY_ORGANIZATION) {
        ui->stationLabel->setText(value.isNull() ? tr("(unknown station)") : value.toString());
    }
    else if (key == MAFW_METADATA_KEY_IS_SEEKABLE) {
        isMediaSeekable = value.toBool();

        if (mafwState == Paused) {
            ui->positionSlider->setEnabled(isMediaSeekable);
        } else if (mafwState == Playing) {
            onStateChanged(Playing);
        }
    }
    else if (key == MAFW_METADATA_KEY_DURATION) {
        ui->streamLengthLabel->setText(mmss_len(value.isNull() ? Duration::Unknown : value.toInt()));
        ui->positionSlider->setRange(0, value.toInt());
    }
    else if (key == MAFW_METADATA_KEY_RENDERER_ART_URI) {
        setAlbumImage(value.isNull() ? defaultRadioImage : value.toString());
    }
    else if (key == MAFW_METADATA_KEY_URI) {
        uri = value.toString();
    }
}

void RadioNowPlayingWindow::onStatusReceived(MafwPlaylist *, uint, MafwPlayState state)
{
    disconnect(mafwRenderer, SIGNAL(statusReceived(MafwPlaylist*,uint,MafwPlayState,QString,QString)),
               this, SLOT(onStatusReceived(MafwPlaylist*,uint,MafwPlayState)));

    connect(mafwRenderer, SIGNAL(stateChanged(MafwPlayState)), this, SLOT(onStateChanged(MafwPlayState)));
    connect(mafwRenderer, SIGNAL(mediaChanged(int,QString)), this, SLOT(onMediaChanged()));

    onStateChanged(state);
    onMediaChanged();
}

void RadioNowPlayingWindow::updateSongLabel()
{
    QString labelText = this->title;

    if (title.isEmpty())
        labelText = tr("(unknown artist)") + " / " + tr("(unknown song)");
    else if (!artist.isEmpty())
        labelText.prepend(artist + " / ");

    ui->songLabel->setText(QFontMetrics(ui->songLabel->font()).elidedText(labelText, Qt::ElideRight, 425));
}

void RadioNowPlayingWindow::onPositionChanged(int position)
{
    ui->currentPositionLabel->setText(mmss_pos(position));

    if (ui->positionSlider->maximum() != 0) {
        if (!ui->positionSlider->isSliderDown())
            ui->positionSlider->setValue(position);
    } else {
        if (ui->positionSlider->value() != 0)
            ui->positionSlider->setValue(0);
    }
}

void RadioNowPlayingWindow::onBufferingInfo(float status)
{
    if (status == 1.0) {
        ui->bufferBar->hide();
        ui->seekWidget->show();
        ui->bufferBar->setRange(0, 0);
    } else { // status != 1.0
        int percentage = (int)(status*100);
        ui->bufferBar->setRange(0, 100);
        ui->bufferBar->setValue(percentage);
        ui->bufferBar->setFormat(tr("Buffering") + " %p%");

        if (ui->bufferBar->isHidden()) {
            ui->seekWidget->hide();
            ui->bufferBar->show();
        }
    }
}

void RadioNowPlayingWindow::onNextButtonClicked()
{
    if (ui->nextButton->isDown()) {
        buttonWasDown = true;
        mafwRenderer->setPosition(SeekRelative, 3);
    } else {
        if (!buttonWasDown)
            mafwRenderer->next();
        buttonWasDown = false;
    }
}

void RadioNowPlayingWindow::onPreviousButtonClicked()
{
    if (ui->prevButton->isDown()) {
        buttonWasDown = true;
        mafwRenderer->setPosition(SeekRelative, -3);
    } else {
        if (!buttonWasDown)
            mafwRenderer->previous();
        buttonWasDown = false;
    }
}

void RadioNowPlayingWindow::onPositionSliderPressed()
{
    onPositionSliderMoved(ui->positionSlider->value());
}

void RadioNowPlayingWindow::onPositionSliderReleased()
{
    ui->currentPositionLabel->setText(mmss_pos(ui->positionSlider->value()));
    mafwRenderer->setPosition(SeekAbsolute, ui->positionSlider->value());
}

void RadioNowPlayingWindow::onPositionSliderMoved(int position)
{
    ui->currentPositionLabel->setText(mmss_pos(position));
    if (!lazySliders)
        mafwRenderer->setPosition(SeekAbsolute, position);
}

void RadioNowPlayingWindow::setAlbumImage(QString image)
{
    qDeleteAll(albumArtScene->items());

    ui->albumArtView->setScene(albumArtScene);
    albumArtScene->setBackgroundBrush(QBrush(Qt::transparent));

    mirror *m = new mirror();
    albumArtScene->addItem(m);

    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(QPixmap(image).scaled(QSize(295, 295),
                                                        Qt::IgnoreAspectRatio,
                                                        Qt::SmoothTransformation));

    albumArtScene->addItem(item);
    m->setItem(item);
}

void RadioNowPlayingWindow::onOrientationChanged(int w, int h)
{
    if (w > h) { // Landscape
        ui->orientationLayout->setDirection(QBoxLayout::LeftToRight);
        ui->volumeWidget->show();
        ui->stationLabel->setAlignment(Qt::AlignLeft);
        ui->songLabel->setAlignment(Qt::AlignLeft);
    } else { // Portrait
        ui->volumeWidget->hide();
        ui->orientationLayout->setDirection(QBoxLayout::TopToBottom);
        ui->stationLabel->setAlignment(Qt::AlignHCenter);
        ui->songLabel->setAlignment(Qt::AlignHCenter);
    }
}

void RadioNowPlayingWindow::showFMTXDialog()
{
    FMTXDialog *fmtxDialog = new FMTXDialog(this);
    fmtxDialog->show();
}

void RadioNowPlayingWindow::showBookmarkDialog()
{
    BookmarkDialog(this, mafwRegistry, Media::Audio, uri, ui->stationLabel->text()).exec();
}

void RadioNowPlayingWindow::showDetails()
{
    (new MetadataDialog(this, MissionControl::acquire()->metadataWatcher()->metadata()))->show();
}

void RadioNowPlayingWindow::onNextButtonPressed()
{
    if (ui->nextButton->isDown())
        ui->nextButton->setIcon(QIcon(nextButtonPressedIcon));
    else
        ui->nextButton->setIcon(QIcon(nextButtonIcon));
}

void RadioNowPlayingWindow::onPrevButtonPressed()
{
    if (ui->prevButton->isDown())
        ui->prevButton->setIcon(QIcon(prevButtonPressedIcon));
    else
        ui->prevButton->setIcon(QIcon(prevButtonIcon));
}

void RadioNowPlayingWindow::onStopButtonPressed()
{
    if (ui->playButton->isDown())
        ui->playButton->setIcon(QIcon(stopButtonPressedIcon));
    else
        ui->playButton->setIcon(QIcon(stopButtonIcon));
}

void RadioNowPlayingWindow::togglePlayback()
{
    if (mafwState == Playing)
        mafwRenderer->pause();
    else if (mafwState == Paused)
        mafwRenderer->resume();
    else if (mafwState == Stopped)
        mafwRenderer->play();
}

void RadioNowPlayingWindow::onPlayMenuRequested(const QPoint &pos)
{
    QMenu *contextMenu = new KbMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Stop playback"), mafwRenderer, SLOT(stop()));
    contextMenu->exec(this->mapToGlobal(pos));
}
