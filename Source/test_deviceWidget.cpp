#include<deviceWidget.h>
#include<device.h>
#include<MyThread.h>
// device类命令行输出测试:
void allocateDevice(string device, string process, DeviceQueue &device_queue, string request = "", int priority = 0){
    bool result = device_queue.allocate_device(device, process, request, priority); 
    if (result) {
        cout << "进程 " << process << " 成功请求到设备 " << device << endl;
    }else {
        cout << "设备类型 " << device << " 不存在" << endl;
    }
}
void runDevice(string device, DeviceQueue &device_queue){
    string process;
    device_queue.release_device(device, process);
    if(process == NOEXIST) {
        cout << "设备 " << device << " 不存在" << endl;
        return;
    }
    if(process == EMPTY){
        cout <<  "设备 " << device << " 当前没有任务" << endl;
        return;
    }
    cout << "设备 " << device << " 已执行进程 " << process << " 的任务" << endl;
}
void printInfo(DeviceQueue &device_queue, DeviceTable &device_table){
    cout << "==== DeviceQueue Status =====" << endl;
    device_queue.print_avaliable_devices();
    device_queue.print_occupied_devices();
    cout << "======= Device Table ========" << endl;
    device_table.printInfo();
    cout << "=============================" << endl;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 创建设备信息表并添加设备
    DeviceTable deviceTable;
    deviceTable.add_device("screen1", "screen", 1);
    deviceTable.add_device("printer1", "printer");
    deviceTable.add_device("printer2", "printer");
    deviceTable.add_device("printer3", "printer");
    deviceTable.add_device("disk1", "disk");
    // 使用设备信息表初始化设备队列
    DeviceQueue deviceQueue(deviceTable);

    // 创建设备管理窗口
    DeviceMainWindow deviceMainWindow(deviceTable, deviceQueue, 1);
    deviceMainWindow.show();

    // 其它线程模拟进程请求设备
    MyThread thread(deviceQueue, deviceMainWindow);
    thread.start();

    return app.exec();
}