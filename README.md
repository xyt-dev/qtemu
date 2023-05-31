# 明天验收

---

## 使用说明

编译步骤：

编辑.pro文件 -> 使用qmake构建 -> make编译  得到release文件夹

按照Qt文档

Qt主程序格式为:

```c++
int main(int argc, char *argv[]){
    QApplication app(argc, argv);
    
    // 创建窗口
    // 其它程序
    
    return app.exec();
}
```
### 进程管理模块接口说明：

### 中断处理模块接口说明：

### 内存管理模块接口说明：

### 文件管理模块接口说明：

### 设备管理模块接口说明：

文件结构：

Source文件夹：源码存放文件

Source/release: 输出文件

Source：

​	device.h: 设备信息表类 和 设备队列类

​	deviceWidget.h: 设备处理模块 及 设备管理窗口和设备窗口

​	MyThread.h: 测试用的线程类

​	test_device: 设备信息表类 和 设备队列类 单元测试

​	test_deviceWidget.cpp: 设备管理模块 模块测试



DeviceModuleTest.exe： 设备管理模块测试 二进制可运行



接下来说明设备管理模块的接口及使用方法：

```c++
// 首先在主程序main中应该创建一个设备信息表并添加设备，如：
// 设备种类有显示器screen,打印机printer,磁盘disk
// add_device(string 设备名字(随意，认得出就行), 设备种类(以上三种之一), 设备优先级(默认为0,数字越大优先级越高))
    DeviceTable deviceTable;
    deviceTable.add_device("screen1", "screen", 1);
    deviceTable.add_device("printer1", "printer");
    deviceTable.add_device("printer2", "printer");
    deviceTable.add_device("printer3", "printer");
    deviceTable.add_device("disk1", "disk");
// 磁盘设备就创建一个就行
// 然后使用该设备信息表初始化一个设备任务队列
 DeviceQueue deviceQueue(deviceTable);
// 然后创建设备管理主窗口，并令其显示 参数(设备信息表，设备队列，是否在命令行输出日志(1为是,0为否))
    DeviceMainWindow deviceMainWindow(deviceTable, deviceQueue, 1);
    deviceMainWindow.show();
```

初始化完成后就可以在其他线程中使用设备管理模块了

这里创建了一个线程，并在该线程中调用使用为例：

```c++
class MyThread : public QThread {
    Q_OBJECT //使用Q_OBJECT 从而能够使用Qt信号与槽通信
private:
    DeviceQueue &deviceQueue;
    DeviceMainWindow &deviceMainWindow;
signals: // 定义两个信号用于启动设备处理，和停止设备处理
    void startSignal();
    void stopSignal();
public:
     MyThread(DeviceQueue &deviceQueue, DeviceMainWindow &deviceMainWindow) : deviceQueue(deviceQueue), deviceMainWindow(deviceMainWindow) {
         // 将该信号连接到deviceMainWindow的start()和stop函数
        connect(this, SIGNAL(startSignal()), &deviceMainWindow, SLOT(start()));
        connect(this, SIGNAL(stopSignal()), &deviceMainWindow, SLOT(stop()));
    }
    void run() override {
        emit stopSignal(); // emit该信号即可时设备处理程序暂停
        emit startSignal();  // 或启动
        // 不需要可以不用，设备处理程序默认启动
        
        // 接口:
        // deviceQueue.allocate_device(string设备种类，string进程名称, string任务信息，int任务在该设备队列中的优先级【且数字越大优先级越高】)
        // 目前支持的任务种类: print任务(只支持 显示器screen，和打印机printer)、read/write任务(只支持 磁盘disk)
        // 任务信息中参数以逗号分隔，磁盘块号范围为0~19，
        // print任务:"print,text(要打印的字符串)"
        // write任务:"write,磁盘快号,距磁盘块首部偏移字节数,text(写入的字符串)"
        // read任务:"read,磁盘快号,距磁盘块首部偏移字节数,读取长度,读取到目标buffer号(1~999)"
        // 示例:
        	// 打印到打印机
            deviceQueue.allocate_device("printer", "p1", "print,p1: hello printer i'm p1");
            deviceQueue.allocate_device("printer", "p2", "print,p2: hello printer i'm p2");
        	// 打印到显示器
            deviceQueue.allocate_device("screen", "p3", "print,p3: hello printer i'm p3", 2);
        	// 读取和写入
            deviceQueue.allocate_device("disk", "p0", "write,0,10,hello disk0.");
        	deviceQueue.allocate_device("disk", "p1", "read,0,10,12,0");
			// 查看读取到的数据
        	if(!deviceQueue.readInBuffer[0].empty())
        		cout << deviceQueue.readInBuffer[0]; // index 为 0~999
       // 如果确定要分配的设备名字，要将任务分配给该指定设备:
       // 使用：deviceQueue._allocate(string设备名, string进程名称, string任务信息, int任务在该设备队列中的优先级【且数字越大优先级越高】)
        // 示例:
        	deviceQueue._allocate_device("HuaWeiPrinter", "p1", "print,p1: hello HuaWeiPrinter");
    }
}
```







---
程序界面：
![原](yuan.png)
