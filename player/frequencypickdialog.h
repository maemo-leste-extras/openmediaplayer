#ifndef FREQUENCYPICKDIALOG_H
#define FREQUENCYPICKDIALOG_H

#include <QDialog>

#include "ui_frequencypickdialog.h"

class FrequencyPickDialog : public QDialog
{
    Q_OBJECT

public:
    FrequencyPickDialog(QWidget *parent, uint min, uint max, uint step, uint frequency);
    ~FrequencyPickDialog();

    static int khzWidth(uint step);

Q_SIGNALS:
    void selected(uint frequency);

private:
    Ui::FrequencyPickDialog *ui;

    uint step;

private Q_SLOTS:
    void onOrientationChanged(int w, int h);
    void accept();
};

#endif // FREQUENCYPICKDIALOG_H
