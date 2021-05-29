#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QThread>

#include "MSA.h"

struct AlignmentInfo {
    int maxLength = 0;
    bool tightConstraints = false;
    QString plainText;
};

class UserInterface : public QMainWindow
{
    Q_OBJECT

public:
    UserInterface(QWidget* parent = nullptr);

};

class AlignmentWindow : public QWidget
{
    Q_OBJECT

public:
    AlignmentWindow(AlignmentInfo& info, QWidget* parent = nullptr);
};

class WorkerThread : public QThread
{
    Q_OBJECT

public:
    WorkerThread::WorkerThread(Input in, QObject* parent = nullptr) : QThread(parent), input(in)
    { }

signals:
    void resultReady(Output output);

protected:
    virtual void run() override;

private:
    Input input;
};
