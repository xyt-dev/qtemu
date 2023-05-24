#include <QThread>
#include <device.h>
#include <deviceWidget.h>
class MyThread : public QThread {
    Q_OBJECT
private:
    DeviceQueue &deviceQueue;
    DeviceMainWindow &deviceMainWindow;
signals:
    void startSignal();
    void stopSignal();
public:
    MyThread(DeviceQueue &deviceQueue, DeviceMainWindow &deviceMainWindow) : deviceQueue(deviceQueue), deviceMainWindow(deviceMainWindow) {
        connect(this, SIGNAL(startSignal()), &deviceMainWindow, SLOT(start()));
        connect(this, SIGNAL(stopSignal()), &deviceMainWindow, SLOT(stop()));
    }
    void run() override {
      // 测试设备处理线程
        int n = 100, m = 100;
        while(n --){
            deviceQueue.allocate_device("printer", "p1", "print,p1: hello printer num:" + to_string(m - n), 1);
            deviceQueue.allocate_device("printer", "p2", "print,p2: hello printer num:" + to_string(m - n), 2);
            deviceQueue.allocate_device("printer", "p3", "print,p3: hello printer num:" + to_string(m - n));
            deviceQueue.allocate_device("printer", "p0", "print,p0: hello printer num:" + to_string(m - n));
            deviceQueue.allocate_device("printer", "p0", "print,p0: hello printer num:" + to_string(m - n));
        }
        // 暂停 3 秒 等待设备处理线程处理任务
        QThread::msleep(3000);
        emit stopSignal(); // 测试暂停设备处理线程
        n = 30, m = 30;
        while(n --){
            deviceQueue.allocate_device("screen", "p1", "print,p1: hello screen num:" + to_string(m - n));
            deviceQueue.allocate_device("screen", "p2", "print,p2: hello screen num:" + to_string(m - n));
            deviceQueue.allocate_device("screen", "p3", "print,p3: hello screen num:" + to_string(m - n));
        }
        // 暂停 5 秒
        QThread::msleep(5000);
        n = 200, m = 200;
        while(n --){
            deviceQueue.allocate_device("screen", "p0", "print,p0: hello screen num:" + to_string(m - n));
            deviceQueue.allocate_device("printer", "p0", "print,p0: hello screen num:" + to_string(m - n));
        }
        emit startSignal(); // 测试启动设备处理线程
        // 磁盘写入测试
        deviceQueue.allocate_device("disk", "p0", "write,0,10,hello disk0.", 3);
        deviceQueue.allocate_device("disk", "p0", "write,1,10,hello disk1.", 3);
        deviceQueue.allocate_device("disk", "p1", "read,0,10,30,0", 3);
        deviceQueue.allocate_device("disk", "p1", "read,1,0,30,1", 3);

        QThread::msleep(10000);
        cout << "buffer0:" << deviceQueue.readInBuffer[0] << endl;
        cout << "buffer1:" << deviceQueue.readInBuffer[1] << endl;
    }
};