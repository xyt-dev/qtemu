#ifndef SYSTEMMANAGER_H
#define SYSTEMMANAGER_H

#include <QObject>

enum InterruptType {
    KeyboardInterrupt,
    DiskInterrupt,
    PrinterInterrupt
};

class SystemManager: public QObject {
    Q_OBJECT

public:
    SystemManager(QObject *parent = nullptr);

signals:
    void handleDeviceInterrupt(InterruptType type);

public slots:
    void handleInterrupt(InterruptType type);

};

#endif // SYSTEMMANAGER_H
