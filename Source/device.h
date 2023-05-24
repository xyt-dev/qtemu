#ifndef DEVICE
#define DEVICE

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

#define FREE 0
#define BUSY 1

#define NOEXIST "noexist"
#define EMPTY "empty"


struct DevRequest {
    string pname;       // 进程名
    string requestStr;  // 任务信息
    int priority;       // 任务优先级
};

// 设备信息表
class DeviceTable {
public:
    struct Device {
        string name;
        string type;
        int status;
        string pname;   // 占用或将占用的进程
        int priority;   // 设备优先级
    };

public:
    int devNum;
    vector<Device> deviceList;

public:
    DeviceTable() : devNum(0) {}

    Device operator[](string name){
        // 根据设备名获取设备信息
        for (vector<Device>::iterator it = deviceList.begin(); it != deviceList.end(); ++ it) {
            if (it->name == name) {
                return *it;
            }
        }
        Device temp {"none", "none", 0, "none"};
        return temp;
    }

    bool add_device(string name, string type, int priority = 0) {
        // 不允许重名设备
        for (vector<Device>::iterator it = deviceList.begin(); it != deviceList.end(); ++ it) {
            if (it->name == name) {
                return false;
            }
        }
        Device new_device {name, type, FREE, "none", priority};
        deviceList.push_back(new_device);
        devNum ++;
        return true;
    }

    bool remove_device(string name) {
        for (vector<Device>::iterator it = deviceList.begin(); it != deviceList.end(); ++ it) {
            if (it->name == name) {
                deviceList.erase(it);
                devNum --;
                return true;
            }
        }
        return false;
    }

    bool change_device_status(string name, int status, string pname) {
        for (vector<Device>::iterator it = deviceList.begin(); it != deviceList.end(); ++ it) {
            if (it->name == name) {
                it->status = status;
                if(status == FREE)
                    it->pname = "none";
                else
                    it->pname = pname;
                return true;
            }
        }
        return false;
    }

    void get_names(vector<string> &names) {
        for (vector<Device>::iterator it = deviceList.begin(); it != deviceList.end(); ++ it) {
            names.push_back(it->name);
        }
    }

    void get_types(vector<string> &types) {
        for (vector<Device>::iterator it = deviceList.begin(); it != deviceList.end(); ++ it) {
            types.push_back((*it).type);
        }
        sort(types.begin(), types.end());
        vector<string>::iterator new_end = unique(types.begin(), types.end());
        types.erase(new_end, types.end());
    }

    int dev_num() {
        return devNum;
    }

    void printInfo() {
        cout << "|DeviceName" << "|Type" << "|priority" << "|Status" << "|process|\n";
        for (auto it : deviceList) {
            cout << "  " << it.name << "  " << it.type << "  " << it.priority << "  " << (it.status ? "busy" : "free") << "  " << it.pname << "  \n";
        }
    }
};


// 设备队列类
class DeviceQueue {
private:
    DeviceTable &deviceTable;                           // 设备信息表

    vector<string> devices;                             // 设备名列表
    vector<string> types;                               // 设备类型列表
    vector<string> available_devices;                   // 可用设备列表
    map<string, vector<DevRequest>> occupied_devices;   // 正在使用的设备字典，键为设备名，值为使用该设备的进程列表

public:
    DeviceQueue(DeviceTable &_deviceTable) : deviceTable(_deviceTable) {
        //初始化空闲设备列表
        deviceTable.get_names(devices);
        for(string device : devices){
            available_devices.push_back(device);
        }
        deviceTable.get_types(types);
        // for (auto it : types) cout << it << " ";

        // 初始化 buffer
        readInBuffer = vector<string>(1000, "");
    }

    // 分配设备给进程 (设备类型，进程名称，任务信息, 任务优先级)
    bool _allocate_device(string device_name, string process_name, string request, int priority) {
        // 如果设备不存在，则返回 false
        vector<string>::iterator it = find(devices.begin(), devices.end(), device_name);
        if (it == devices.end()) {
            return false;
        }

        // 如果设备已经被使用，则将进程添加到设备的使用列表中
        if (occupied_devices.find(device_name) != occupied_devices.end()) {
            if(priority == 0 || occupied_devices[device_name].size() == 1) // 默认优先级或只有一个处理中的任务，直接添加到队列尾部
                occupied_devices[device_name].push_back(DevRequest{process_name, request, priority});
            else{
                // 非默认优先级，添加到队列中合适的位置
                vector<DevRequest>& processes = occupied_devices[device_name];
                auto it = processes.begin() + 1;
                for(; it != processes.end(); ++ it){
                    if(it->priority < priority){
                        processes.insert(it, DevRequest{process_name, request, priority});
                        break;
                    }
                }
                if(it == processes.end()) // 全为同一种高优先级，it到达队尾
                    processes.push_back(DevRequest{process_name, request, priority});
            }
        }
        // 如果设备未被使用，则创建一个新的使用列表并添加进程，并将该设备移出空闲列表
        else {
            it = find(available_devices.begin(), available_devices.end(), device_name);
            available_devices.erase(it);
            occupied_devices[device_name] = vector<DevRequest>{};
            occupied_devices[device_name].push_back(DevRequest{process_name, request, priority});
            deviceTable.change_device_status(device_name, BUSY, process_name);
        }
        return true;
    }

    // 分配设备给进程 (设备类型，进程名称，任务信息, 任务优先级)
    bool allocate_device(string device_type, string process_name, string request = "", int priority = 0) {
        // 不存在该类型设备
        vector<string>::iterator itt = find(types.begin(), types.end(), device_type);
        if (itt == types.end()) {
            return false;
        }
        // 存在空闲设备
        for(auto it = available_devices.begin(); it != available_devices.end(); ++ it) {
            if(deviceTable[*it].type == device_type) {
                return _allocate_device(*it, process_name, request, priority);
            }
        }

        // 不存在空闲设备，找到任务队列最短设备
        long long unsigned int len = 99999;
        string device_name;
        auto it = occupied_devices.begin();
        for(; it != occupied_devices.end(); ++ it) {
            if(deviceTable[(*it).first].type == device_type && (*it).second.size() < len) {
                len = (*it).second.size();
                device_name = (*it).first;
            }
        }

        return _allocate_device(device_name, process_name, request, priority);
    }

    // 释放设备 (设备名称, 确认结束任务对应进程名，备用)
    bool release_device(string device_name, string &process_name) {
        // 如果设备不存在，返回 false
        vector<string>::iterator it = find(devices.begin(), devices.end(), device_name);
        if (it == devices.end()) {
            process_name = NOEXIST;
            return false;
        }

        // 如果该设备未被占用，返回 flase
        if (occupied_devices.find(device_name) == occupied_devices.end()){
            process_name = EMPTY;
            return false;
        }

        // 将进程从设备的使用列表中删除
        vector<DevRequest>& processes = occupied_devices[device_name];
        process_name = (*processes.begin()).pname;
        processes.erase(processes.begin());

        // 如果设备的使用列表为空，则将设备添加回空闲设备列表中
        if (processes.empty()) {
            available_devices.push_back(device_name);
            occupied_devices.erase(device_name);
            deviceTable.change_device_status(device_name, FREE, "");
        } else {
            deviceTable.change_device_status(device_name, BUSY, (*processes.begin()).pname);
        }
        return true;
    }

    // 获取可用设备列表
    vector<string> &get_available_devices() {
        return available_devices;
    }

    // 获取正在使用设备的字典
    map<string, vector<DevRequest>> &get_occupied_devices() {
        return occupied_devices;
    }

    // 打印可用设备列表
    void print_avaliable_devices() {
        vector<string> available_devices = get_available_devices();
        cout << "Available Devices: ";
        for (string device_name : available_devices) {
            cout << device_name << " ";
        }
        cout << '\n';
    }

    // 打印正在使用设备的字典
    void print_occupied_devices() {
        map<string, vector<DevRequest>> occupied_devices = get_occupied_devices();
        cout << "Occupied Devices: " << '\n';
        for (auto& pair : occupied_devices) {
            string device_name = pair.first;
            vector<DevRequest> processes = pair.second;
            cout << device_name << ": ";
            for (DevRequest process : processes) {
                cout << process.pname << "[\"" << process.requestStr << "\"" << ":" << process.priority << "]" << " ";
            }
            cout << '\n';
        }
    }

public:
    // 设置1000个buffer
    vector<string> readInBuffer;
};

#endif