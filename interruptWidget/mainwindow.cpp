#include "mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_label(new QLabel(this)),
      m_button(new QPushButton("模拟中断", this)),
      m_timer(new QTimer(this)),
      m_systemManager(new SystemManager(this))
{
    setWindowTitle("中断演示程序");

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    m_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_label);

    m_button->setFixedSize(120, 40);
    layout->addWidget(m_button, 0, Qt::AlignCenter);

    connect(m_button, &QPushButton::clicked, this, &MainWindow::handleButtonClicked);

    //建立计时器
    connect(m_timer, &QTimer::timeout, m_systemManager, [=]{
        emit m_systemManager->handleDeviceInterrupt(KeyboardInterrupt);
    });
    m_timer->start(1000);
}

MainWindow::~MainWindow()
{
}

void MainWindow::handleButtonClicked()
{
    //模拟中断
    m_systemManager->handleInterrupt(static_cast<InterruptType>(qrand() % 3));
}
