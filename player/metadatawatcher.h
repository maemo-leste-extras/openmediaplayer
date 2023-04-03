#ifndef METADATAWATCHER_H
#define METADATAWATCHER_H

#include "mafw/mafwregistryadapter.h"
#include "mafw/mafwutils.h"

#include <QObject>
#include <QImage>
#include <QCryptographicHash>

#include "includes.h"

#include <QObject>

class MetadataWatcher: public QObject
{
    Q_OBJECT

public:
    MetadataWatcher(MafwRegistryAdapter *mafwRegistry);

    MafwSourceAdapter* currentSource();
    QMap<QString,QVariant> metadata();

Q_SIGNALS:
    void metadataReady();
    void metadataChanged(QString key, QVariant value);

private:
    MafwRegistryAdapter *mafwRegistry;
    MafwRendererAdapter *mafwRenderer;
    MafwSourceAdapter *mafwSource;
    MafwSourceAdapter *mafwTrackerSource;

    QMap<QString,QVariant> currentMetadata;
    QMap<QString,QVariant> backupMetadata;
    QString currentObjectId;

    bool sourceMetadataPresent;

    void setMetadataFromSource(QString key, QVariant value);
    void setMetadataFromRenderer(QString key, QVariant value);

    static QVariant toQVariant(GValue *v);

private Q_SLOTS:
    void onStatusReceived(MafwPlaylist *, uint index, MafwPlayState, QString objectId);

    void onMediaChanged(int, QString objectId);
    void onSourceMetadataReceived(QString objectId, GHashTable *metadata);
    void onSourceMetadataChanged(QString objectId);
    void onRendererMetadataReceived(GHashTable *metadata, QString objectId);
    void onRendererMetadataChanged(QString metadata, QVariant value);
};

#endif // METADATAWATCHER_H
