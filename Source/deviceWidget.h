#ifndef DEVICE_WIDGET
#define DEVICE_WIDGET
#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QString>
#include <QTimer>
#include <QMap>
#include <QThread>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QStyle>

#include <fstream>

#include <device.h>

const int DISK_BLOCK_SIZE = 4096;  //定义磁盘块大小 (4096 Byte)
const int DISK_BLOCK_NUM = 20;     //定义磁盘块数量 (20 Blocks)

const int MAX_DEVICE_PRIORITY = 10; // 最大优先级 running_v2

#define SUCCESS 0
#define FILE_ERR 1
#define OVERSTEP 2

// 设备类
class DeviceWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit DeviceWindow(QString deviceType = "none", QString deviceName = "none", QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Device: " + deviceName);
        centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        layout = new QVBoxLayout(centralWidget);

        // 显示部件
        printerWidget = new QTextEdit(centralWidget);
        layout->addWidget(printerWidget);
        // 状态标签
        statusLabel = new QLabel("Status: Free", this);
        statusLabel->setAlignment(Qt::AlignBottom | Qt::AlignRight);
        layout->addWidget(statusLabel);

        layout->setAlignment(statusLabel, Qt::AlignBottom | Qt::AlignRight);
    }

    void setStatus(bool isBusy) {
        statusLabel->setText("Status: " + QString(isBusy ? "Busy" : "Free"));
        if(isBusy) statusLabel->setStyleSheet("background-color: rgba(31, 31, 31, 180); color: rgb(241, 76, 76);");
        else statusLabel->setStyleSheet("background-color: rgba(31, 31, 31, 180); color: rgb(230, 230, 230);");
    }

    void print(const QString& text) {
        printerWidget->insertPlainText(text + '\n');
        printerWidget->moveCursor(QTextCursor::End);
        printerWidget->verticalScrollBar()->setValue(printerWidget->verticalScrollBar()->maximum());
    }

    int writeToFile(int blockIndex, int byteIndex, string text) {
        if(blockIndex < 0 || blockIndex >= DISK_BLOCK_NUM) return OVERSTEP; // 磁盘块索引越界
        if(byteIndex < 0 || byteIndex + text.size() >= DISK_BLOCK_SIZE) return OVERSTEP; // 字节索引越界
        ofstream outfile("release/Disk/block" + to_string(blockIndex) + ".txt");
        if(!outfile.is_open()) return FILE_ERR; // 文件打开失败
        outfile.seekp(byteIndex, ios::beg); // 定位到第byteIndex个字节  ios::beg: 文件开头
        outfile << text; // 写入text
        outfile.close();
        return SUCCESS;
    }

    int readFromFile(int blockIndex, int byteIndex, int length, string &text) {
        if(blockIndex < 0 || blockIndex >= DISK_BLOCK_NUM) return OVERSTEP; // 磁盘块索引越界
        if(byteIndex < 0 || byteIndex + length >= DISK_BLOCK_SIZE) return OVERSTEP; // 字节索引越界
        ifstream infile("release/Disk/block" + to_string(blockIndex) + ".txt");
        if(!infile.is_open()) return FILE_ERR; // 文件打开失败
        infile.seekg(byteIndex, ios::beg); // 定位到第byteIndex个字节  ios::beg: 文件开头
        text.clear(); // 清空text
        text.resize(length); // 重置text大小
        infile.read(&text[0], length); // 读取length个字节
        infile.close();
        return SUCCESS;
    }    

public:
    QWidget *centralWidget; // 中心窗口
    QVBoxLayout *layout; // 布局管理器
    QTextEdit *printerWidget; // 打印机组件
    QLabel *statusLabel; // 状态标签

};


// 主窗口类
class DeviceMainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit DeviceMainWindow(DeviceTable &deviceTable, DeviceQueue &deviceQueue, int logger = 0, QWidget *parent = nullptr)
     : QMainWindow(parent), deviceTable(deviceTable), deviceQueue(deviceQueue), logger(logger) {
        setWindowTitle("DeviceWindow");
        centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        layout = new QVBoxLayout(centralWidget);
        statusLabel = new QLabel(centralWidget);
        layout->addWidget(statusLabel);

        // 创建设备窗口
        createDeviceWindows();
        updateDeviceStatus();

        // 初始化设备指针
        occupied_devices = deviceQueue.get_occupied_devices();
        devicePointer = occupied_devices.begin();

        // 初始化运行记录 running_v2
        resetRunningRecord();

        // 设置定时器
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(running()));
        timer->start(10); // 处理频率 10ms

    }

    void enableLogger() {logger = 1;}
    void disableLogger() {logger = 0;}

    void resetRunningRecord() {
        for(int i = 0; i <= MAX_DEVICE_PRIORITY; ++ i) {
            running_record[i] = pow(2, i);
        }
    }

    string argi(string request, int index){ // 请求的第 i 个参数
        int i = 1;
        int start = 0;
        int end = request.find(",");
        while(i < index){
            start = request.find(",", start) + 1;
            end = request.find(",", start);
            ++ i;
        }
        return request.substr(start, end - start);
    }

public slots:

void updateDeviceStatus() {
    // 更新设备状态显示
    QString statusText;
    vector<Device> deviceList = deviceTable.get_device_list();
    for (const auto &device : deviceList) {
        QString deviceStatus = device.status ? "Busy" : "Free";
        QString statusLine = QString("DeviceName: %1  |  Type: %2  |  DevicePriority: %3  |  Status: %4  |  Process: %5")
                                .arg(QString::fromStdString(device.name))
                                .arg(QString::fromStdString(device.type))
                                .arg(device.priority)
                                .arg(deviceStatus)
                                .arg(QString::fromStdString(device.pname));

        // 创建带样式的文本
        QString styledLine = device.status ? ("<font color='red'>" + statusLine + "</font>") : statusLine;

        statusText += styledLine + "<br>";  // 添加换行符

        if (device.type == "screen") {
            // 更新屏幕设备窗口的状态
            screenWindows[device.name]->setStatus(device.status);
        } else if (device.type == "printer") {
            // 更新打印机设备窗口的状态
            printerWindows[device.name]->setStatus(device.status);
        } else if (device.type == "disk") {
            // 更新磁盘设备窗口的状态
            diskWindows[device.name]->setStatus(device.status);
        }
    }
    statusLabel->setText(statusText);
}

    void running(){ // running_v2
        // 更新occupied_devices
        if(devicePointer == occupied_devices.end()){
            occupied_devices = deviceQueue.get_occupied_devices();
            devicePointer = occupied_devices.begin();
            // 更新max_priority
            int _max_priority = 0;
            for(auto device : occupied_devices){
                int priority = deviceTable[device.first].priority;
                if(priority > _max_priority) _max_priority = priority;
            }
            max_priority = _max_priority;
            // 防饿死机制 running_v2
            while(running_record[max_priority] == 0){
                -- max_priority;
                if(max_priority == -1) {
                    resetRunningRecord();
                    max_priority = _max_priority;
                }
                // else if(logger) cout << "防饿死机制: 优先级 " << max_priority + 1 << " 的设备不执行\n";
            }
            -- running_record[max_priority];
        }
        // 遍历occupied_devices
        while(devicePointer != occupied_devices.end() && deviceTable[devicePointer->first].priority != max_priority)
            ++ devicePointer; // 优先级不是最高的设备不执行
        if(devicePointer == occupied_devices.end()) return;
        string device_name = devicePointer->first;
        vector<DevRequest> requests = devicePointer->second;
        string process_name = requests[0].pname;
        string request = requests[0].requestStr;
        // screen
        if(deviceTable[device_name].type == "screen"){
            // request = "print,text"
            // 屏幕打印
            if(request.find("print") != string::npos){
                if(logger) cout << "设备 " << device_name << " 执行进程 " << process_name << " 的任务:[" << request << "]\n";
                screenWindows[device_name]->print(QString::fromStdString(argi(request, 2)));
            }
        // printer
        }else if(deviceTable[device_name].type == "printer"){
            // request = "print,text"
            // 打印机打印
            if(request.find("print") != string::npos){
                if(logger) cout << "设备 " << device_name << " 执行进程 " << process_name << " 的任务:[" << request << "]\n";
                printerWindows[device_name]->print(QString::fromStdString(argi(request, 2)));
            }
        // disk
        }else if(deviceTable[device_name].type == "disk"){
            if (request.find("write") != std::string::npos) {
                // 写入磁盘
                // request = "write,blockIndex,byteIndex,text"
                int blockIndex = std::stoi(argi(request, 2));
                int byteIndex = std::stoi(argi(request, 3));
                string text = argi(request, 4);

                int flag;
                if ((flag = diskWindows[device_name]->writeToFile(blockIndex, byteIndex, text)) == SUCCESS) {
                    if (logger)
                        std::cout << "设备 " << device_name << " 执行进程 " << process_name << " 的任务:[" << request << "]\n";

                    // 在窗口中显示任务信息
                    QString taskInfo = "Write Task - Block: " + QString::number(blockIndex) +
                                    ", Byte: " + QString::number(byteIndex) +
                                    ", Text: " + QString::fromStdString(text);
                    diskWindows[device_name]->print(taskInfo);
                } else if (flag == OVERSTEP) {
                    // 处理越界情况
                    if (logger)
                        std::cout << "设备 " << device_name << " 执行进程 " << process_name << " 的任务越界:[" << request << "]\n";

                    // 在窗口中显示越界信息
                    QString errorInfo = "Error: OverStep - Block: " + QString::number(blockIndex) +
                                        ", Byte: " + QString::number(byteIndex) +
                                        ", Text: " + QString::fromStdString(text);
                    diskWindows[device_name]->print(errorInfo);
                } else if (flag == FILE_ERR) {
                    // 处理文件打开失败情况
                    if (logger)
                        std::cout << "设备 " << device_name << " 执行进程 " << process_name << " 的任务文件打开失败:[" << request << "]\n";

                    // 在窗口中显示文件打开失败信息
                    QString errorInfo = "Error: File Open Failed - Block: " + QString::number(blockIndex) +
                                        ", Byte: " + QString::number(byteIndex) +
                                        ", Text: " + QString::fromStdString(text);
                    diskWindows[device_name]->print(errorInfo);
                }
            }else if (request.find("read") != std::string::npos) {
                // 读取磁盘
                // request = "read,blockIndex,byteIndex,length,buffernum"
                int blockIndex = std::stoi(argi(request, 2));
                int byteIndex = std::stoi(argi(request, 3));
                int length = std::stoi(argi(request, 4));
                int buffernum = std::stoi(argi(request, 5));
                
                int flag;
                if((flag = diskWindows[device_name]->readFromFile(blockIndex, byteIndex, length, deviceQueue.readInBuffer[buffernum])) == SUCCESS){
                    if(logger)
                        std::cout << "设备 " << device_name << " 执行进程 " << process_name << " 的任务:[" << request << "]\n";

                    // 在窗口中显示任务信息
                    QString taskInfo = "Read Task - Block: " + QString::number(blockIndex) +
                                    ", Byte: " + QString::number(byteIndex) +
                                    ", Length: " + QString::number(length) +
                                    ", Content: " + QString::fromStdString(deviceQueue.readInBuffer[buffernum]);
                    diskWindows[device_name]->print(taskInfo);
                }else if(flag == OVERSTEP){
                    // 处理越界情况
                    if(logger)
                        std::cout << "设备 " << device_name << " 执行进程 " << process_name << " 的任务越界:[" << request << "]\n";

                    // 在窗口中显示越界信息
                    QString errorInfo = "Error: OverStep - Block: " + QString::number(blockIndex) +
                                        ", Byte: " + QString::number(byteIndex) +
                                        ", Length: " + QString::number(length);
                    diskWindows[device_name]->print(errorInfo);
                }else if(flag == FILE_ERR){
                    // 处理文件打开失败情况
                    if(logger)
                        std::cout << "设备 " << device_name << " 执行进程 " << process_name << " 的任务文件打开失败:[" << request << "]\n";

                    // 在窗口中显示文件打开失败信息
                    QString errorInfo = "Error: File Open Failed - Block: " + QString::number(blockIndex) +
                                        ", Byte: " + QString::number(byteIndex) +
                                        ", Length: " + QString::number(length);
                    diskWindows[device_name]->print(errorInfo);
                }

            }
        } // disk

        // 释放设备
        deviceQueue.release_device(device_name, process_name);
        ++ devicePointer;
        updateDeviceStatus(); // 更新设备状态显示
    }    

    void stop() {
        timer->stop();
    }

    void start() {
        timer->start(10);
    }


private:
    void createDeviceWindows() {
        QPalette palette;
        int x = 0, y = 0;
        for (const auto &device : deviceTable.deviceList) {
            if (device.type == "screen") {
                // 创建屏幕设备窗口
                DeviceWindow *screenWindow = new DeviceWindow("screen", QString::fromStdString(device.name), this);
                screenWindow->setStyleSheet("background-color: rgba(31, 31, 31, 180); color: rgb(230, 230, 230);");
                screenWindow->move(x, y);
                x += screenWindow->width();
                y += screenWindow->height();
                screenWindows[device.name] = screenWindow;
                screenWindow->show();
            } else if (device.type == "printer") {
                // 创建打印机设备窗口
                DeviceWindow *printerWindow = new DeviceWindow("printer", QString::fromStdString(device.name), this);
                printerWindow->setStyleSheet("background-color: rgba(31, 31, 31, 180); color: rgb(230, 230, 230);");
                printerWindow->move(x, y);
                x += printerWindow->width();
                y += printerWindow->height();
                printerWindows[device.name] = printerWindow;
                printerWindow->show();
            } else if (device.type == "disk") {
                // 创建磁盘设备窗口
                DeviceWindow *diskWindow = new DeviceWindow("disk", QString::fromStdString(device.name), this);
                diskWindow->setStyleSheet("background-color: rgba(31, 31, 31, 180); color: rgb(230, 230, 230);");
                diskWindow->move(x, y);
                x += diskWindow->width();
                y += diskWindow->height();
                diskWindows[device.name] = diskWindow;
                diskWindow->show();
            }
        }
    }

    int logger = 0;

    // running遍历使用
    map<string, vector<DevRequest>>::iterator devicePointer; // 设备指针
    map<string, vector<DevRequest>> occupied_devices; // 临时设备字典
    int max_priority = 0; // 最高优先级
    map<int, int> running_record; // 运行记录

    QWidget *centralWidget;
    QVBoxLayout *layout;
    QLabel *statusLabel;
    QTimer *timer;
    DeviceTable &deviceTable; // 引用设备信息表
    DeviceQueue &deviceQueue; // 引用设备队列
    QMap<string, DeviceWindow*> screenWindows; // 屏幕设备窗口
    QMap<string, DeviceWindow*> printerWindows; // 打印机设备窗口
    QMap<string, DeviceWindow*> diskWindows; // 磁盘设备窗口
};

#endif