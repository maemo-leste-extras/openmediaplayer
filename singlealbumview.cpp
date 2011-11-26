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

#include "singlealbumview.h"

SingleAlbumView::SingleAlbumView(QWidget *parent, MafwAdapterFactory *factory) :
    QMainWindow(parent),
    ui(new Ui::SingleAlbumView)
#ifdef MAFW
    ,mafwFactory(factory),
    mafwrenderer(factory->getRenderer()),
    mafwTrackerSource(factory->getTrackerSource()),
    playlist(factory->getPlaylistAdapter())
#endif
{
    ui->setupUi(this);
    QString shuffleText(tr("Shuffle songs"));
    ui->centralwidget->setLayout(ui->verticalLayout);

#ifdef MAFW
    ui->indicator->setFactory(mafwFactory);
    this->isSingleAlbum = false;
#endif

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    shuffleAllButton = new QMaemo5ValueButton(shuffleText, this);
    shuffleAllButton->setValueLayout(QMaemo5ValueButton::ValueUnderTextCentered);
    ui->searchHideButton->setIcon(QIcon::fromTheme("general_close"));
#else
    shuffleAllButton = new QPushButton(shuffleText, this);
#endif
    SingleAlbumViewDelegate *delegate = new SingleAlbumViewDelegate(ui->songList);
    ui->songList->setItemDelegate(delegate);

    ui->songList->setContextMenuPolicy(Qt::CustomContextMenu);

    shuffleAllButton->setIcon(QIcon::fromTheme(defaultShuffleIcon));
    ui->verticalLayout->removeWidget(ui->songList);
    ui->verticalLayout->removeWidget(ui->searchWidget);
    ui->verticalLayout->addWidget(shuffleAllButton);
    ui->verticalLayout->addWidget(ui->songList);
    ui->verticalLayout->addWidget(ui->searchWidget);

    connect(ui->songList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onItemSelected(QListWidgetItem*)));
    connect(ui->songList->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->indicator, SLOT(poke()));
    connect(ui->songList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(shuffleAllButton, SIGNAL(clicked()), this, SLOT(onShuffleButtonClicked()));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    connect(ui->searchEdit, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));
    connect(ui->searchHideButton, SIGNAL(clicked()), this, SLOT(onSearchHideButtonClicked()));
    connect(ui->actionAdd_to_now_playing, SIGNAL(triggered()), this, SLOT(addAllToNowPlaying()));
    connect(ui->actionDelete, SIGNAL(triggered()), this, SLOT(deleteCurrentAlbum()));
#ifdef MAFW
    connect(mafwTrackerSource, SIGNAL(containerChanged(QString)), this, SLOT(onContainerChanged(QString)));
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
            this, SLOT(browseAllSongs(uint, int, uint, QString, GHashTable*, QString)));
#endif
    this->orientationChanged();
}

SingleAlbumView::~SingleAlbumView()
{
    delete ui;
}

#ifdef MAFW
void SingleAlbumView::listSongs()
{
#ifdef DEBUG
    qDebug() << "SingleAlbumView: Source ready";
#endif

    ui->songList->clear();

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator);
#endif

    this->browseAllSongsId = mafwTrackerSource->sourceBrowse(this->albumObjectId.toUtf8(), true, NULL, "+track,+title",
                                                             MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                                              MAFW_METADATA_KEY_ALBUM,
                                                                              MAFW_METADATA_KEY_ARTIST,
                                                                              MAFW_METADATA_KEY_ALBUM_ART_URI,
                                                                              MAFW_METADATA_KEY_DURATION,
                                                                              MAFW_METADATA_KEY_TRACK),
                                                             0, MAFW_SOURCE_BROWSE_ALL);
}

void SingleAlbumView::browseAllSongs(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString)
{
    if (browseId != browseAllSongsId)
      return;

    if (metadata != NULL) {
        QString title;
        QString artist;
        QString album;
        int duration;
        int trackNumber;
        GValue *v;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_TITLE);
        title = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown song)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ARTIST);
        artist = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown artist)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM);
        album = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown album)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_DURATION);
        duration = v ? g_value_get_int (v) : Duration::Unknown;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_TRACK);
        trackNumber = v ? g_value_get_int (v) : -1;

        QListWidgetItem *item = new QListWidgetItem(ui->songList);
        item->setText(title);
        item->setData(UserRoleSongTitle, title);
        item->setData(UserRoleSongArtist, artist);
        item->setData(UserRoleSongAlbum, album);
        item->setData(UserRoleObjectID, objectId);
        item->setData(UserRoleSongDuration, duration);

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM_ART_URI);
        if (v != NULL) {
            const gchar* file_uri = g_value_get_string(v);
            gchar* filename = NULL;
            if(file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL) {
                item->setData(UserRoleAlbumArt, QString::fromUtf8(filename));
            }
        }

        // Although we don't need this to show the song title, we need it to
        // sort alphabatically.
        item->setText(title);
        ui->songList->addItem(item);
    }

#ifdef Q_WS_MAEMO_5
    shuffleAllButton->setValueText(tr("%n song(s)", "", ui->songList->count()));

    if (remainingCount == 0)
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
}

#ifdef MAFW
void SingleAlbumView::browseAlbumByObjectId(QString objectId)
{
    this->albumObjectId = objectId;
    if (mafwTrackerSource->isReady())
        this->listSongs();
    else
        connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listSongs()));
}
#endif

void SingleAlbumView::onItemSelected(QListWidgetItem *item)
{
#ifdef MAFW
    playlist->assignAudioPlaylist();
#endif

    this->createPlaylist(false);

    playlist->getSize(); // explained in musicwindow.cpp
    mafwrenderer->gotoIndex(ui->songList->row(item));
    mafwrenderer->play();
}

#endif


void SingleAlbumView::orientationChanged()
{
    ui->songList->scroll(1,1);

    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55), 112, 70);
    ui->indicator->raise();
}

void SingleAlbumView::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace)
        this->close();
}

void SingleAlbumView::onShuffleButtonClicked()
{
    this->createPlaylist(true);
}

void SingleAlbumView::createPlaylist(bool shuffle)
{
#ifdef MAFW
    if (ui->songList->count() > 0) {
        this->setEnabled(false);

#ifdef DEBUG
        qDebug() << "Clearing playlist";
#endif
        playlist->clear();
        playlist->setShuffled(shuffle);
#ifdef DEBUG
        qDebug() << "Playlist cleared";
#endif

        int songCount = ui->songList->count();
        gchar** songAddBuffer = new gchar*[songCount+1];

        for (int i = 0; i < songCount; i++)
            songAddBuffer[i] = qstrdup(ui->songList->item(i)->data(UserRoleObjectID).toString().toUtf8());
        songAddBuffer[songCount] = NULL;

        playlist->appendItems((const gchar**)songAddBuffer);

        for (int i = 0; i < songCount; i++)
            delete[] songAddBuffer[i];
        delete[] songAddBuffer;

        if (shuffle) {
            playlist->getSize(); // explained in musicwindow.cpp
            mafwrenderer->play();
        }

#ifdef DEBUG
        qDebug() << "Playlist created";
#endif

        NowPlayingWindow *window = NowPlayingWindow::acquire(this, mafwFactory);

        window->show();

        connect(window, SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
        ui->indicator->inhibit();
    }
#endif
}

void SingleAlbumView::onSearchHideButtonClicked()
{
    if (ui->searchEdit->text().isEmpty()) {
        ui->searchWidget->hide();
        ui->indicator->restore();
    } else
        ui->searchEdit->clear();
}

void SingleAlbumView::onSearchTextChanged(QString text)
{
    for (int i = 0; i < ui->songList->count(); i++) {
        if (ui->songList->item(i)->text().toLower().indexOf(text.toLower()) == -1)
            ui->songList->item(i)->setHidden(true);
        else
            ui->songList->item(i)->setHidden(false);
    }

    if (text.isEmpty()) {
        ui->searchWidget->hide();
        ui->indicator->restore();
    }
}

void SingleAlbumView::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Left || e->key() == Qt::Key_Right || e->key() == Qt::Key_Backspace)
        return;
    else if ((e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) && !ui->searchWidget->isHidden())
        ui->songList->setFocus();
    else {
        ui->songList->clearSelection();
        if (ui->searchWidget->isHidden()) {
            ui->indicator->inhibit();
            ui->searchWidget->show();
        }
        if (!ui->searchEdit->hasFocus())
            ui->searchEdit->setText(ui->searchEdit->text() + e->text());
        ui->searchEdit->setFocus();
    }
}

void SingleAlbumView::addAllToNowPlaying()
{
    if (ui->songList->count() > 0) {
#ifdef MAFW
        if (playlist->playlistName() == "FmpVideoPlaylist" || playlist->playlistName() == "FmpRadioPlaylist")
            playlist->assignAudioPlaylist();

        int songCount = ui->songList->count();
        gchar** songAddBuffer = new gchar*[songCount+1];

        for (int i = 0; i < songCount; i++)
            songAddBuffer[i] = qstrdup(ui->songList->item(i)->data(UserRoleObjectID).toString().toUtf8());
        songAddBuffer[songCount] = NULL;

        playlist->appendItems((const gchar**)songAddBuffer);

        for (int i = 0; i < songCount; i++)
            delete[] songAddBuffer[i];
        delete[] songAddBuffer;

#ifdef Q_WS_MAEMO_5
        this->notifyOnAddedToNowPlaying(songCount);
#endif

#endif
    }
}

void SingleAlbumView::onContextMenuRequested(const QPoint &point)
{
    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Add to now playing"), this, SLOT(onAddToNowPlaying()));
    contextMenu->addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
    contextMenu->addAction(tr("Set as ringing tone"), this, SLOT(setRingingTone()));
    contextMenu->addAction(tr("Share"), this, SLOT(onShareClicked()));
    contextMenu->exec(point);
}

void SingleAlbumView::onAddToNowPlaying()
{
#ifdef MAFW
    if (playlist->playlistName() == "FmpVideoPlaylist" || playlist->playlistName() == "FmpRadioPlaylist")
        playlist->assignAudioPlaylist();

    playlist->appendItem(ui->songList->currentItem()->data(UserRoleObjectID).toString());

#ifdef Q_WS_MAEMO_5
    this->notifyOnAddedToNowPlaying(ui->songList->selectedItems().count());
#endif

#endif
}

void SingleAlbumView::setRingingTone()
{
#ifdef MAFW
    QMessageBox confirmDelete(QMessageBox::NoIcon,
                              " ",
                              tr("Are you sure you want to set this song as ringing tone?")+ "\n\n"
                              + ui->songList->currentItem()->data(UserRoleSongTitle).toString() + "\n"
                              + ui->songList->currentItem()->data(UserRoleSongArtist).toString(),
                              QMessageBox::Yes | QMessageBox::No,
                              this);
    confirmDelete.button(QMessageBox::Yes)->setText(tr("Yes"));
    confirmDelete.button(QMessageBox::No)->setText(tr("No"));
    confirmDelete.exec();
    if (confirmDelete.result() == QMessageBox::Yes) {
        mafwTrackerSource->getUri(ui->songList->currentItem()->data(UserRoleObjectID).toString().toUtf8());
        connect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onRingingToneUriReceived(QString,QString)));
    }
#endif
    ui->songList->clearSelection();
}

#ifdef MAFW
void SingleAlbumView::onRingingToneUriReceived(QString objectId, QString uri)
{
    disconnect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onRingingToneUriReceived(QString,QString)));

    if (objectId != ui->songList->currentItem()->data(UserRoleObjectID).toString())
        return;

#ifdef Q_WS_MAEMO_5
    QDBusInterface setRingtone("com.nokia.profiled",
                               "/com/nokia/profiled",
                               "com.nokia.profiled",
                               QDBusConnection::sessionBus(), this);
    setRingtone.call("set_value", "general", "ringing.alert.tone", uri);
    QMaemo5InformationBox::information(this, "Selected song set as ringing tone");
#endif
}
#endif

void SingleAlbumView::onShareClicked()
{
#ifdef MAFW
    mafwTrackerSource->getUri(ui->songList->currentItem()->data(UserRoleObjectID).toString().toUtf8());
    connect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));
#endif
}

#ifdef MAFW
void SingleAlbumView::onShareUriReceived(QString objectId, QString uri)
{
    disconnect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));

    if (objectId != ui->songList->currentItem()->data(UserRoleObjectID).toString())
        return;

    QStringList list;
    QString clip;
    clip = uri;

    list.append(clip);
#ifdef Q_WS_MAEMO_5
    Share *share = new Share(this, list);
    share->setAttribute(Qt::WA_DeleteOnClose);
    share->show();
#endif
}
#endif

#ifdef MAFW
void SingleAlbumView::onContainerChanged(QString objectId)
{
    if (objectId == "localtagfs::music")
        this->listSongs();
}
#endif

void SingleAlbumView::onDeleteClicked()
{
#ifdef MAFW
    QMessageBox confirmDelete(QMessageBox::NoIcon,
                              " ",
                              tr("Delete selected item from device?"),
                              QMessageBox::Yes | QMessageBox::No,
                              this);
    confirmDelete.button(QMessageBox::Yes)->setText(tr("Yes"));
    confirmDelete.button(QMessageBox::No)->setText(tr("No"));
    confirmDelete.exec();
    if (confirmDelete.result() == QMessageBox::Yes) {
        mafwTrackerSource->destroyObject(ui->songList->currentItem()->data(UserRoleObjectID).toString().toUtf8());
        delete ui->songList->currentItem();
    }
#endif
    ui->songList->clearSelection();
}

void SingleAlbumView::deleteCurrentAlbum()
{
#ifdef MAFW
    QMessageBox confirmDelete(QMessageBox::NoIcon,
                              " ",
                              tr("Delete all items shown in view?"),
                              QMessageBox::Yes | QMessageBox::No,
                              this);
    confirmDelete.button(QMessageBox::Yes)->setText(tr("Yes"));
    confirmDelete.button(QMessageBox::No)->setText(tr("No"));
    confirmDelete.exec();
    if (confirmDelete.result() == QMessageBox::Yes) {
        mafwTrackerSource->destroyObject(albumObjectId.toUtf8());
        this->close();
    }
#endif
}

#ifdef Q_WS_MAEMO_5
void SingleAlbumView::notifyOnAddedToNowPlaying(int songCount)
{
    QMaemo5InformationBox::information(this, tr("%n clip(s) added to now playing", "", songCount));
}
#endif

void SingleAlbumView::onNowPlayingWindowHidden()
{
    disconnect(NowPlayingWindow::acquire(), SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
    ui->indicator->restore();
    ui->songList->clearSelection();
    this->setEnabled(true);
}
