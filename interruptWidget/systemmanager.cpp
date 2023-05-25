#include "systemmanager.h"
#include <QDebug>

SystemManager::SystemManager(QObject *parent) : QObject(parent)
{
    // 这里可以加上其它初始化逻辑
}

void SystemManager::handleInterrupt(InterruptType type)
{
    if (type == KeyboardInterrupt) {
        QString message = tr("键盘中断");
        qDebug() << message;
    }
    else if (type == DiskInterrupt) {
        QString message = tr("磁盘中断");
        qDebug() << message;
    }
    else if (type == PrinterInterrupt) {
        QString message = tr("打印机中断");
        qDebug() << message;
    }
}
