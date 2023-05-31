#include <QThread>
#include <device.h>
#include <deviceWidget.h>
class TestThread : public QThread {
    Q_OBJECT
public:
    DeviceQueue &deviceQueue;
    DeviceMainWindow &deviceMainWindow;
signals:
    void startSignal();
    void stopSignal();
public:
    TestThread(DeviceQueue &deviceQueue, DeviceMainWindow &deviceMainWindow) : deviceQueue(deviceQueue), deviceMainWindow(deviceMainWindow) {
        connect(this, SIGNAL(startSignal()), &deviceMainWindow, SLOT(start()));
        connect(this, SIGNAL(stopSignal()), &deviceMainWindow, SLOT(stop()));
    }
    void run() override {
    // 测试分配给指定设备
        deviceQueue._allocate_device("MyScreen", "TestProcess", "print,测试分配给指定设备:printer1");
        int n = 100, m = 100;
        while(n --){
            deviceQueue._allocate_device("printer1", "p1", "print,p1: hello printer num:" + to_string(m - n));
            deviceQueue._allocate_device("printer1", "p2", "print,p2: hello printer num:" + to_string(m - n));
            deviceQueue._allocate_device("printer1", "p3", "print,p3: hello printer num:" + to_string(m - n));
        }
        QThread::msleep(5000);
    // 测试设备内任务队列防饿死机制
        deviceQueue._allocate_device("MyScreen", "TestProcess", "print,\n测试设备内任务队列防饿死机制:printer2");
        emit stopSignal(); // 暂停设备处理
        // 初始分配 100 个高优先级任务 和 100 个低优先级任务
        n = 100, m = 100;
        while(n --){
            deviceQueue._allocate_device("printer2", "p1", "print,p1: higher priority num:" + to_string(m - n), 1); // 较高优先级任务
            deviceQueue._allocate_device("printer2", "p2", "print,p2: lower priority num:" + to_string(m - n), 0); // 较低优先级任务
        }
        emit startSignal(); // 启动设备处理
        deviceQueue._allocate_device("MyScreen", "TestProcess", "print,已分配到printer2: 100个较高优先级任务, 100个较低优先级任务");
        QThread::msleep(5000);
    // 测试任务优先机制
        deviceQueue._allocate_device("MyScreen", "TestProcess", "print,\n测试设备内任务队列优先级机制:printer3");
        n = 100, m = 100;
        while(n --){
            deviceQueue._allocate_device("printer3", "p1", "print,p1: p1(high) task num:" + to_string(m - n), 10); // 高优先级任务
            deviceQueue._allocate_device("printer3", "p2", "print,p2: p2(low) task num:" + to_string(m - n)); // 低优先级任务
            deviceQueue._allocate_device("printer3", "p3", "print,p3: p3(low) task num:" + to_string(m - n)); // 低优先级任务
            deviceQueue._allocate_device("printer3", "p4", "print,p4: p4(low) task num:" + to_string(m - n)); // 低优先级任务
        }
        deviceQueue._allocate_device("MyScreen", "TestProcess", "print,已分配到printer3: p1 100个很高优先级任务, p2,p3,p4 各100个低优先级任务");
        QThread::msleep(10000);
    // 测试同种类设备间任务分配
        deviceQueue._allocate_device("MyScreen", "TestProcess", "print,\n测试同种类设备间任务分配机制:all printer");
        n = 500, m = 500;
        while(n --){
            deviceQueue.allocate_device("printer", "p1", "print,p1: printer task num:" + to_string(m - n));
        }
        QThread::msleep(10000);
    // 测试不同优先级设备间防饿死机制
        deviceQueue._allocate_device("MyScreen", "TestProcess", "print,\n测试不同优先级设备间防饿死机制:between screen and printer");
        n = 500, m = 500;
        while(n --){
            deviceQueue.allocate_device("printer", "p1", "print,p1: print to printer task num:" + to_string(m - n));
            deviceQueue._allocate_device("screen1", "p1", "print,p1: print to screen task num:" + to_string(m - n));
        }
        QThread::msleep(10000);
    // 磁盘设备读写测试
        deviceQueue._allocate_device("MyScreen", "TestProcess", "print,\n测试磁盘设备读写:disk");

        deviceQueue.allocate_device("disk", "p0", "write,0,10,write to block0.", 3);
        deviceQueue.allocate_device("disk", "p1", "read,0,10,30,0", 3);
        QThread::msleep(1000);
        deviceQueue._allocate_device("screen1", "p1", "print,从块0偏移10字节处读取30字节:");
        string readInStr = deviceQueue.readInBuffer[0];
        deviceQueue._allocate_device("screen1", "p1", "print," + readInStr);

        deviceQueue.allocate_device("disk", "p0", "write,1,10,write to block1.", 3);
        deviceQueue.allocate_device("disk", "p1", "read,1,0,30,1", 3);
        QThread::msleep(1000);
        deviceQueue._allocate_device("screen1", "p1", "print,从块1偏移0字节处读取30字节:");
        readInStr = deviceQueue.readInBuffer[1];
        deviceQueue._allocate_device("screen1", "p1", "print," + readInStr);

        deviceQueue.allocate_device("disk", "p0", "write,0,10,hello", 3);
        deviceQueue.allocate_device("disk", "p1", "read,0,10,30,2", 3);
        QThread::msleep(1000);
        deviceQueue._allocate_device("screen1", "p1", "print,从块0偏移10字节处读取30字节:");
        readInStr = deviceQueue.readInBuffer[2];
        deviceQueue._allocate_device("screen1", "p1", "print," + readInStr);

    }
};