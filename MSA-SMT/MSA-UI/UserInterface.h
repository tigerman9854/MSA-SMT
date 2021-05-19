#pragma once

#include <QMainWindow>
#include <QWidget>

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