#include "rotator.h"

Rotator* Rotator::instance = NULL;

Rotator* Rotator::acquire()
{
    return instance ? instance : instance = new Rotator();
}

Rotator::Rotator() : m_slave(NULL), m_policy(Automatic)
{
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(onResized()));
}

void Rotator::setPolicy(Orientation policy)
{
    m_policy = policy;

    if (m_slave) switch (m_policy) {
        case Automatic:
            m_slave->setProperty("X-Maemo-Orientation", 2);
            break;
        case Landscape:
            m_slave->setProperty("X-Maemo-Orientation", 0);
            break;
        case Portrait:
            m_slave->setProperty("X-Maemo-Orientation", 1);
            break;
    }

    onResized();
}

void Rotator::setSlave(QWidget *slave)
{
    m_slave = slave;
}

void Rotator::addClient(QObject *client)
{
    connect(this, SIGNAL(rotated(int,int)), client, SLOT(onOrientationChanged(int,int)));

    QMetaObject::invokeMethod(client, "onOrientationChanged", Q_ARG(int, w), Q_ARG(int, h));
}

Rotator::Orientation Rotator::policy()
{
    return m_policy;
}

int Rotator::width()
{
    return w;
}

int Rotator::height()
{
    return h;
}

void Rotator::onResized()
{
    QRect screen = QApplication::desktop()->screenGeometry();

    switch (m_policy) {
        case Portrait:
            // Looks like the window is unconditionally rotatated to landscape
            // whenever entering the task navigator in landscape, so instead of
            // stubbornly drawing the window for portrait (with an ugly result),
            // automatic orientation should be used in this case.
        case Automatic:
            w = screen.width();
            h = screen.height();
            break;
        case Landscape:
            w = qMax(screen.width(), screen.height());
            h = qMin(screen.width(), screen.height());
            break;
    }

    emit rotated(w, h);
}
