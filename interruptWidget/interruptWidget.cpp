#include"interruptWidget.h"

InterruptWidget::InterruptWidget(QWidget *parent)
    : QWidget(parent){
    init();}

InterruptWidget::~InterruptWidget(){
    delete label;
    delete button;
    delete vLayout;
    delete hLayout;
    delete timer;}

void InterruptWidget::init(){
    // 初始化界面元素

    label = new QLabel("Click the button to generate an interrupt.", this);
    button = new QPushButton("Generate Interrupt", this);
    vLayout = new QVBoxLayout(this);
    hLayout = new QHBoxLayout();

    hLayout->addStretch();
    hLayout->addWidget(button);
    hLayout->addStretch();

    vLayout->addWidget(label);
    vLayout->addLayout(hLayout);

    // 初始化计时器

    timer = new QTimer(this);
    count = 0;

    // 初始化设备信息表和任务队列

    deviceTable.add_device("screen1", "screen", 1);
    deviceTable.add_device("printer1", "printer");
    deviceTable.add_device("printer2", "printer");
    deviceTable.add_device("printer3", "printer");
    deviceTable.add_device("disk1", "disk");
    deviceQueue = DeviceQueue(deviceTable);

    // 初始化设备管理主窗口

    deviceMainWindow = new DeviceMainWindow(deviceTable, deviceQueue, 0);

    // 连接信号槽

    connect(button, &QPushButton::clicked, this, &InterruptWidget::onButtonClicked);
    connect(timer, &QTimer::timeout, this, &InterruptWidget::onTimeout);

    // 启动计时器

    timer->start(1000);}

void InterruptWidget::onButtonClicked(){
    // 模拟进程执行异常时发出的中断

    qDebug() << "Exception occurred, generating interrupt...";
    deviceQueue.interrupt(0); // 向设备管理模块发出外部中断

    deviceQueue.suspend_process(1); // 对进程管理模块发出挂起进程指令
}

void InterruptWidget::onTimeout(){
    // 模拟计时器中断

    count++;
    qDebug() << "Timer interrupt occurred, count:" << count;
    deviceQueue.interrupt(1); // 向设备管理模块发出外部中断
}