#include "upnpview.h"

const QString AudioMime = MAFW_METADATA_VALUE_MIME_AUDIO;
const QString VideoMime = MAFW_METADATA_VALUE_MIME_VIDEO;

UpnpView::UpnpView(QWidget *parent, MafwRegistryAdapter *mafwRegistry, MafwSourceAdapter *source) :
    BrowserWindow(parent, mafwRegistry),
    mafwRegistry(mafwRegistry),
    mafwSource(source),
    playlist(mafwRegistry->playlist())
{
    ui->objectList->setIconSize(QSize(48, 48));
    ui->objectList->setItemDelegate(new MediaWithIconDelegate(ui->objectList));

    objectProxyModel->setFilterRole(UserRoleTitle);

    ui->windowMenu->addAction(tr("Add songs to now playing"), this, SLOT(addAllToNowPlaying()));

    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Enter), this), SIGNAL(activated()), this, SLOT(onContextMenuRequested()));

    connect(ui->objectList, SIGNAL(activated(QModelIndex)), this, SLOT(onItemActivated(QModelIndex)));
    connect(ui->objectList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
}

UpnpView::~UpnpView()
{
    // Last window in the stack should delete the source
    if (!qobject_cast<UpnpView*>(this->parentWidget()))
        delete mafwSource;
}

void UpnpView::browseObjectId(QString objectId)
{
    setProperty("X-Maemo-Progress", 1);

    objectModel->clear();

    connect(mafwSource, SIGNAL(browseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(onBrowseResult(uint,int,uint,QString,GHashTable*)), Qt::UniqueConnection);

    browseId = mafwSource->browse(objectId, true, NULL, NULL,
                                  MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                   MAFW_METADATA_KEY_DURATION,
                                                   MAFW_METADATA_KEY_MIME,
                                                   MAFW_METADATA_KEY_URI,
                                                   MAFW_METADATA_KEY_PROTOCOL_INFO),
                                  0, MAFW_SOURCE_BROWSE_ALL);
}

void UpnpView::onBrowseResult(uint browseId, int remainingCount, uint, QString objectId, GHashTable *metadata)
{
    if (browseId != this->browseId) return;

    if (metadata != NULL) {
        GValue *v;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_TITLE);
        QString title = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown song)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_MIME);
        QString mime = v ? QString::fromUtf8(g_value_get_string(v)) : QString();

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_URI);
        QString uri = v ? QString::fromUtf8(g_value_get_string(v)) : QString();

        if (mime != MAFW_METADATA_VALUE_MIME_CONTAINER) {
            // UPnP source does not always report correct MIME under the proper
            // key, so try to obtain it by other means.
            v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_PROTOCOL_INFO);

            if (v) {
                // Protocol info value should start with something like this:
                // "http-get:*:audio/mpeg:DLNA.ORG_PN=MP3;"
                QString info = QString::fromUtf8(g_value_get_string(v));

                int colon2 = info.indexOf(':', info.indexOf(':') + 1);

                if (colon2 != -1 && info.length() - colon2 > 5) {
                    QStringRef infoMime(&info, colon2+1, 5);

                    if (!infoMime.compare(QString("audio"))) {
                        mime = AudioMime;
                    } else if (!infoMime.compare(QString("video"))) {
                        mime = VideoMime;
                    } else {
                        v = NULL;
                    }
                } else {
                    v = NULL;
                }
            }

            if (!v) {
                if (mime.startsWith("audio")) {
                    mime = AudioMime;
                } else if (mime.startsWith("video")) {
                    mime = VideoMime;
                }
            }
        }

        int duration;
        if (mime == AudioMime || mime == VideoMime) {
            v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_DURATION);
            duration = v ? g_value_get_int(v) : Duration::Unknown;
        } else {
            duration = Duration::Blank;
        }

        QStandardItem *item = new QStandardItem();

        item->setData(objectId, UserRoleObjectID);
        item->setData(duration, UserRoleSongDuration);
        item->setData(title, UserRoleTitle);
        item->setData(mime, UserRoleMIME);
        item->setData(uri, UserRoleURI);

        if (mime == MAFW_METADATA_VALUE_MIME_CONTAINER)
            item->setIcon(QIcon::fromTheme("general_folder"));
        else if (mime == AudioMime)
            item->setIcon(QIcon::fromTheme("general_audio_file"));
        else if (mime == VideoMime)
            item->setIcon(QIcon::fromTheme("general_video_file"));
        else
            item->setIcon(QIcon::fromTheme("filemanager_unknown_file"));

        objectModel->appendRow(item);
    }

    if (remainingCount == 0) {
        disconnect(mafwSource, SIGNAL(browseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(onBrowseResult(uint,int,uint,QString,GHashTable*)));
        setProperty("X-Maemo-Progress", 0);
    }
}

void UpnpView::onContextMenuRequested(const QPoint &pos)
{
    if (ui->objectList->currentIndex().data(UserRoleMIME).toString() == MAFW_METADATA_VALUE_MIME_CONTAINER) return;

    QMenu *contextMenu = new KbMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    if (ui->objectList->currentIndex().data(UserRoleMIME).toString() == AudioMime) {
        contextMenu->addAction(tr("Add to now playing"), this, SLOT(onAddOneToNowPlaying()));
        contextMenu->addAction(tr("Add to a playlist"), this, SLOT(onAddOneToPlaylist()));
    }
    contextMenu->addAction(tr("Open in web browser"), this, SLOT(openCurrentUri()));
    contextMenu->addAction(tr("Details"), this, SLOT(showDetails()));
    contextMenu->exec(this->mapToGlobal(pos));
}

void UpnpView::onItemActivated(QModelIndex index)
{
    QString objectId = index.data(UserRoleObjectID).toString();
    QString mime = index.data(UserRoleMIME).toString();

    if (mime == MAFW_METADATA_VALUE_MIME_CONTAINER) {
        this->setEnabled(false);
        UpnpView *window = new UpnpView(this, mafwRegistry, mafwSource);
        window->browseObjectId(objectId);
        window->setWindowTitle(index.data(UserRoleTitle).toString());
        window->show();

        connect(window, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
        ui->indicator->inhibit();

    } else if (mime == AudioMime) {
        this->setEnabled(false);
        playlist->assignAudioPlaylist();
        playlist->clear();
        playlist->setShuffled(false);

        bool filter = QSettings().value("main/playlistFilter", false).toBool();

        appendAllToPlaylist(AudioMime, filter);

        int sameTypeIndex = 0;
        if (filter) {
            int selectedRow = index.row();
            for (int i = 0; i < selectedRow; i++)
                if (objectProxyModel->index(i,0).data(UserRoleMIME).toString() == AudioMime)
                    ++sameTypeIndex;
        } else {
            int selectedRow = objectProxyModel->mapToSource(index).row();
            for (int i = 0; i < selectedRow; i++)
                if (objectModel->item(i)->data(UserRoleMIME).toString() == AudioMime)
                    ++sameTypeIndex;
        }

        MafwRendererAdapter *mafwRenderer = mafwRegistry->renderer();
        mafwRenderer->gotoIndex(sameTypeIndex);
        mafwRenderer->play();

        NowPlayingWindow *window = NowPlayingWindow::acquire(this, mafwRegistry);
        window->show();

        connect(window, SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
        ui->indicator->inhibit();

    } else if (mime == VideoMime) {
        this->setEnabled(false);
        playlist->assignVideoPlaylist();
        playlist->clear();

        bool filter = QSettings().value("main/playlistFilter", false).toBool();

        appendAllToPlaylist(VideoMime, filter);

        int sameTypeIndex = 0;
        if (filter) {
            int selectedRow = index.row();
            for (int i = 0; i < selectedRow; i++)
                if (objectProxyModel->index(i,0).data(UserRoleMIME).toString() == VideoMime)
                    ++sameTypeIndex;
        } else {
            int selectedRow = objectProxyModel->mapToSource(index).row();
            for (int i = 0; i < selectedRow; i++)
                if (objectModel->item(i)->data(UserRoleMIME).toString() == VideoMime)
                    ++sameTypeIndex;
        }

        VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this, mafwRegistry);
        window->showFullScreen();

        connect(window, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
        ui->indicator->inhibit();

        MafwRendererAdapter *mafwRenderer = mafwRegistry->renderer();
        mafwRenderer->gotoIndex(sameTypeIndex);
        window->play();

    } else {
        openCurrentUri();
    }
}

void UpnpView::onAddOneToNowPlaying()
{
    playlist->assignAudioPlaylist();
    playlist->appendItem(ui->objectList->currentIndex().data(UserRoleObjectID).toString());

    notifyOnAddedToNowPlaying(1);
}

void UpnpView::onAddOneToPlaylist()
{
    PlaylistPicker picker(this);
    picker.exec();
    if (picker.result() == QDialog::Accepted) {
        MafwPlaylistAdapter(picker.playlistName).appendItem(ui->objectList->currentIndex().data(UserRoleObjectID).toString());
        QMaemo5InformationBox::information(this, tr("%n clip(s) added to playlist", "", 1));
    }
}

void UpnpView::openCurrentUri()
{
    QDesktopServices::openUrl(ui->objectList->currentIndex().data(UserRoleURI).toString());
}

void UpnpView::showDetails()
{
    (new MetadataDialog(this, mafwSource, ui->objectList->currentIndex().data(UserRoleObjectID).toString()))->show();
}

void UpnpView::addAllToNowPlaying()
{
    playlist->assignAudioPlaylist();

    notifyOnAddedToNowPlaying(appendAllToPlaylist(AudioMime, true));
}

int UpnpView::appendAllToPlaylist(QString type, bool filter)
{
    int visibleCount = filter ? objectProxyModel->rowCount() : objectModel->rowCount();
    gchar** itemAddBuffer = new gchar*[visibleCount+1];

    int sameTypeCount = 0;
    if (filter) {
        for (int i = 0; i < visibleCount; i++)
            if (objectProxyModel->index(i,0).data(UserRoleMIME).toString() == type)
                itemAddBuffer[sameTypeCount++] = qstrdup(objectProxyModel->index(i,0).data(UserRoleObjectID).toString().toUtf8());
    } else {
        for (int i = 0; i < visibleCount; i++)
            if (objectModel->item(i)->data(UserRoleMIME).toString() == type)
                itemAddBuffer[sameTypeCount++] = qstrdup(objectModel->item(i)->data(UserRoleObjectID).toString().toUtf8());
    }

    itemAddBuffer[sameTypeCount] = NULL;

    playlist->appendItems((const gchar**) itemAddBuffer);

    for (int i = 0; i < sameTypeCount; i++)
        delete[] itemAddBuffer[i];
    delete[] itemAddBuffer;

    return sameTypeCount;
}

void UpnpView::notifyOnAddedToNowPlaying(int songCount)
{
    QMaemo5InformationBox::information(this, tr("%n clip(s) added to now playing", "", songCount));
}

void UpnpView::onNowPlayingWindowHidden()
{
    disconnect(NowPlayingWindow::acquire(), SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));

    this->onChildClosed();
}
