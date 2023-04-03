#ifndef SLEEPER_H
#define SLEEPER_H

#include "mafw/mafwrendereradapter.h"

#include <QObject>

#include <QTimer>
#include <QDateTime>
#include <QApplication>
#include <QSettings>
#include <QtCore>
#include <QDebug>

class Sleeper : public QObject
{
    Q_OBJECT

public:
    enum VolumeReduction
    {
        NoReduction = 0,
        LinearReduction,
        ExponentialReduction
    };

    Sleeper(QObject *parent, MafwRendererAdapter *mafwRenderer);

    qint64 end();

    void start(int seconds, int reduction);
    void stop();

private:
    MafwRendererAdapter *mafwRenderer;

    QTimer *masterTimer;
    QTimer *volumeTimer;
    qint64 startStamp;
    qint64 endStamp;
    int reduction;
    int volume;

    void scheduleVolumeStep(int volume);

Q_SIGNALS:
    void finished();

private Q_SLOTS:
    void onInitialVolumeReceived(int volume);
    void onPropertyChanged(const QString &name, const QVariant &value);
    void stepVolume();
    void onTimeout();
};

#endif // SLEEPER_H
