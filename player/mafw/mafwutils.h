#ifndef MAFWUTILS_H
#define MAFWUTILS_H

#include <glib-object.h>
#include <QString>
#include <QVariant>

namespace MafwUtils
{
    QString toQString(const GError *e);
    QVariant toQVariant(const GValue *v);
}

#endif // MAFWUTILS_H
