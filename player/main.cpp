/**************************************************************************
    Open MediaPlayer
    Copyright (C) 2010-2013 Mohammad Abu-Garbeyyeh
                            Grzegorz Gidel
                            Matias Perez
                            Pali Rohár
                            Nicolai Hess
                            Timur Kristof

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

#include <glib.h>
#include <gio/gio.h>
#include <libmafw/mafw-log.h>
#include <libmafw/mafw-registry.h>
#include "mafw/mafwplaylistadapter.h"
#include "mafw/mafwregistryadapter.h"
#include "mafw/mafwutils.h"

static MafwRendererSeekMode a = MafwRendererSeekMode::SeekAbsolute;
static GSettingsBindFlags b = GSettingsBindFlags::G_SETTINGS_BIND_DEFAULT;

#include <QObject>
#include <QtWidgets/QApplication>
#include <QTime>
#include <QTextStream>
#include <QTranslator>

#include "mainwindow.h"

QByteArray fileOpen(const QString &path) {
  QFile file(path);
  if(!file.open(QFile::ReadOnly | QFile::Text)) {
    return QByteArray();
  }

  QByteArray data = file.readAll();
  file.close();
  return data;
}

QString barrayToString(const QByteArray &data) {
  return QString(QTextCodec::codecForMib(106)->toUnicode(data));
}

int main(int argc, char *argv[]) {
#ifdef DEBUG
  // For remote debugging (QEMU) with CLion, the environment variables need
  // to be correctly set such that e.g. dbus will work. We can execute
  // this hack to dump the environ to a file, then reads it below.
  // This snippet runs when `-D CMAKE_DEBUG_TYPE=Debug`

  // /bin/sh -c 'nohup /tmp/tmp.fCTLrJeUgW/cmake-build-debug/bin/conversations >/dev/null 2>&1 &'; sleep 2; cat "/proc/`pidof conversations`/environ" | tr "\0" "\n" > /home/user/env.sh; kill -9 "`pidof conversations`"
  setuid(1000);
  QString path_env_file = "/home/user/env.sh";
  qDebug() << "trying to read ENV from" << path_env_file << ", if it exists";
  auto env_file = fileOpen(path_env_file);
  for(auto &line: barrayToString(env_file).split("\n")) {
    line = line.replace("export ", "");
    int pos = line.indexOf("=");
    auto key = line.left(pos);
    auto val = line.remove(0, pos + 1);

    if(val.startsWith("\""))
      val = val.mid(1);
    if(val.endsWith("\""))
      val = val.mid(0, val.length() - 1);

    if(val.isEmpty() || key.isEmpty()) continue;
    qputenv(key.toStdString().c_str(), val.toStdString().c_str());
  }
#endif

    QApplication::setOrganizationName("openmediaplayer");
    QApplication::setApplicationName("openmediaplayer");
    QApplication::setApplicationVersion("0.1");
    QApplication a(argc, argv);

    if (!QDBusConnection::sessionBus().isConnected())
        qWarning("Cannot connect to the D-Bus session bus.");

    if (!(argc > 1 && QString(argv[1]) == "--allow-multiple-instances")
    && QDBusConnection::sessionBus().interface()->isServiceRegistered(DBUS_SERVICE)) {
        qWarning("Mediaplayer already running.");
        QDBusConnection::sessionBus().send(QDBusMessage::createMethodCall(DBUS_SERVICE, DBUS_PATH, DBUS_INTERFACE, "top_application"));
        return 0;
    }

    QString langPath = "/opt/openmediaplayer/lang/";
    QString lang = qgetenv("LANG");
    QLocale::setDefault(lang);
    qDebug() << "Detected language:" << lang;

    // Install language file
    QTranslator translator;
    if (translator.load("openmediaplayer." + lang, langPath)) {
        qDebug() << "Translator successfully loaded";
        a.installTranslator(&translator);
    } else {
        qDebug() << "Translator failed to load";
        qDebug() << "Falling back to English";
        if (translator.load("openmediaplayer.en", langPath)) {
            qDebug() << "Translator successfully loaded";
            a.installTranslator(&translator);
        } else
            qDebug() << "Translator failed to load";
    }

    // Disable mafw syslog logging
    mafw_log_init(NULL);

    QTime t(0,0);
    t.start();
    // TODO: Add a full list of contributors here when we're ready to release.
    /*QTextStream out(stdout);
    out << "Open MediaPlayer, version: " << QApplication::applicationVersion() << " "
        << "Running with PID: " << QApplication::applicationPid() << endl
        << "Copyright (C) 2010-2011 Mohammad Abu-Garbeyyeh" << endl
        << "Licensed under GPLv3" << endl
        << "This program comes with ABSOLUTELY NO WARRANTY" << endl
        << "This is free software, and you are welcome to redistribute it" << endl
        << "under certain conditions; visit http://www.gnu.org/licenses/gpl.txt for details." << endl;*/
    MainWindow w;

#if defined(Q_WS_S60)
    w.showMaximized();
#else
    w.show();
#endif
    //out << QString("MediaPlayer startup took %1 ms").arg(t.elapsed()) << endl;
    return a.exec();
}
