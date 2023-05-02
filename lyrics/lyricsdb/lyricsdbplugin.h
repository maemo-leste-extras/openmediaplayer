#ifndef LYRICSDBPLUGIN_H
#define LYRICSDBPLUGIN_H

#include "abstractlyricsprovider.h"
#include <QtPlugin>
#include <QXmlStreamReader>
#include <QTextDocument>

class LyricsDBPlugin : public AbstractLyricsProvider
{
    Q_OBJECT
    Q_INTERFACES(AbstractLyricsProvider)
    Q_PLUGIN_METADATA(IID "org.maemo.mediaplayer.lyrics.db")

public:
    LyricsDBPlugin();

    QString name() { return "LyricsDB"; }
    QString description() { return "http://lyrics.mirkforce.net"; }

    void fetch(QString artist, QString title);
    void abort();

private slots:
    void onReplyReceived();
};

#endif // LYRICSDBPLUGIN_H
