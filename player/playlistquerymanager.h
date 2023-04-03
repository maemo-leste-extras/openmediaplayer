#ifndef PLAYLISTQUERYMANAGER_H
#define PLAYLISTQUERYMANAGER_H

#include "mafw/mafwplaylistadapter.h"

#include "includes.h"
#include <QObject>
#include <QList>
#include <QDebug>

class PlaylistQueryManager : public QObject
{
    Q_OBJECT

public:
    PlaylistQueryManager(QObject *parent, MafwPlaylistAdapter *mafwPlaylist);
    ~PlaylistQueryManager();
    void getItems(int first, int last);
    void itemsInserted(int from, int amount);
    void itemsRemoved(int from, int amount);

Q_SIGNALS:
    void gotItem(QString objectId, GHashTable *metadata, uint index);

public Q_SLOTS:
    void setPriority(int position);

private:
    void mergeRequest(int first, int last);
    void queryPlaylist();
    void restart();

    MafwPlaylistAdapter *mafwPlaylist;
    QList<int*> requests;
    gpointer getItemsOp;
    int priority;
    int* rangeInProgress;

private Q_SLOTS:
    void onItemReceived(QString objectId, GHashTable *metadata, uint index, gpointer op);
    void onRequestComplete(gpointer op);
};

#endif // PLAYLISTQUERYMANAGER_H
