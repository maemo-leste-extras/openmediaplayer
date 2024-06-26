#include "sharedialog.h"

ShareDialog::ShareDialog(QWidget *parent, MafwSourceAdapter *mafwSource, QString objectId) :
    QDialog(parent),
    ui(new Ui::ShareDialog),
    objectId(objectId),
    shareAction(NULL)
{
    ui->setupUi(this);

    this->setAttribute(Qt::WA_DeleteOnClose);
    // This is a workaround for Qt::WA_Maemo5ShowProgressIndicator not being
    // properly handled when enabled from the constructor.
    QTimer::singleShot(0, this, SLOT(setProgressIndicator()));

    connect(ui->bluetoothButton, SIGNAL(clicked()), this, SLOT(onBluetoothClicked()));
    connect(ui->emailButton, SIGNAL(clicked()), this, SLOT(onEmailClicked()));

    connect(mafwSource, SIGNAL(gotUri(QString,QString,QString)), this, SLOT(onUriReceived(QString,QString)));

    mafwSource->getUri(objectId);
}

ShareDialog::~ShareDialog()
{
    delete ui;
}

void ShareDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace)
        this->close();
}

void ShareDialog::setProgressIndicator()
{
    setProperty("X-Maemo-Progress", 1);
}

void ShareDialog::onUriReceived(QString objectId, QString uri)
{
    if (objectId != this->objectId) return;

    disconnect(this->sender(), SIGNAL(gotUri(QString,QString,QString)), this, SLOT(onUriReceived(QString,QString)));

    setProperty("X-Maemo-Progress", 0);

    this->uri = uri;

    if (shareAction)
        (this->*shareAction)();
}

void ShareDialog::onBluetoothClicked()
{
    if (uri.isNull()) {
        this->setEnabled(false);
        shareAction = &ShareDialog::shareViaBluetooth;
    } else {
        shareViaBluetooth();
    }
}

void ShareDialog::onEmailClicked()
{
    if (uri.isNull()) {
        this->setEnabled(false);
        shareAction = &ShareDialog::shareViaEmail;
    } else {
        shareViaEmail();
    }
}

void ShareDialog::shareViaBluetooth()
{
    QStringList files(uri);

    QDBusInterface("com.nokia.icd_ui",
                   "/com/nokia/bt_ui",
                   "com.nokia.bt_ui",
                   QDBusConnection::systemBus())
    .call(QDBus::NoBlock, "show_send_file_dlg", files);

    this->close();
}

void ShareDialog::shareViaEmail()
{
    QDBusInterface("com.nokia.modest",
                   "/com/nokia/modest",
                   "com.nokia.modest",
                   QDBusConnection::sessionBus())
    .call(QDBus::NoBlock, "ComposeMail",
          QString(), // to
          QString(), // cc
          QString(), // bcc
          QString(), // subject
          QString(), // body
          QString(uri));

    this->close();
}
