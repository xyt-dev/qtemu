#define INTERRUPTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QDebug>
#include "device.h"
#include "deviceWidget.h"

class InterruptWidget : public QWidget
{
    Q_OBJECT

public:
    InterruptWidget(QWidget *parent = nullptr);
    ~InterruptWidget();
private:
    QLabel *label;
    QPushButton *button;
    QVBoxLayout *vLayout;
    QHBoxLayout *hLayout;

    QTimer *timer; // 计时器

    int count; // 计数器


    DeviceTable deviceTable;
    DeviceQueue deviceQueue; // 设备任务队列

    DeviceMainWindow *deviceMainWindow; // 设备管理主窗口


    void init(); // 初始化界面及相关操作

private slots:
    void onButtonClicked(); // 模拟进程执行异常时发出的中断

    void onTimeout(); // 模拟计时器中断
};
#endif // INTERRUPTWIDGET_H