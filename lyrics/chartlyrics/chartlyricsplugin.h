#ifndef CHARTLYRICSPLUGIN_H
#define CHARTLYRICSPLUGIN_H

#include "abstractlyricsprovider.h"
#include <QtPlugin>
#include <QXmlStreamReader>

class ChartLyrics : public AbstractLyricsProvider
{
    Q_OBJECT
    Q_INTERFACES(AbstractLyricsProvider)

public:
    ChartLyrics();

    QString name() { return "ChartLyrics"; }

    void fetch(QString artist, QString title);
    void abort();

private slots:
    void onReplyReceived();
};

#endif // CHARTLYRICSPLUGIN_H
