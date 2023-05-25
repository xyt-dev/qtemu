#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QLabel>
#include<QVBoxLayout>
#include<QPushButton>
#include<QTimer>

class QLabel;
class QPushButton;
class QTimer;

#include "systemmanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QLabel *m_label;
    QPushButton *m_button;

    QTimer *m_timer;

    SystemManager *m_systemManager;

private slots:
    void handleButtonClicked();
};

#endif // MAINWINDOW_H
