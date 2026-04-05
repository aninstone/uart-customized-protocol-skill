#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QMutex>
#include <QTimer>
#include <QLineEdit>
#include <QPushButton>
#include "protocolhandler.h"
#include "serialportmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 串口操作
    void on_refreshPorts();
    void on_openClosePort();

    // 手动控制
    void on_readSend();
    void on_writeSend();
    void on_hexSend();
    void on_buildFrame();

    // 协议配置
    void on_protocolConfigChanged();

    // 电源参数页面
    void on_powerWriteP1();
    void on_powerWriteP2();
    void on_powerWriteP3();
    void on_powerWriteP4();
    void on_powerWriteP5();
    void on_powerWriteP6();
    void on_powerWriteP7();
    void on_powerWriteP8();
    void on_powerWriteP9();
    void on_powerWriteP10();
    void on_powerReadAll();
    void on_powerWriteAll();

    // 运行监控
    void on_monitorStart();
    void on_monitorStop();
    void on_monitorRefresh();

    // 数据处理
    void on_serialDataReceived(const QByteArray &data);
    void on_dataReceived(quint8 cmd, const QByteArray &data, quint8 status);
    void on_rawFrameReceived(const QByteArray &frame);
    void on_errorOccurred(const QString &error);
    void on_connectionStateChanged(SerialPortStatus status);

    // 显示控制
    void on_clearLog();
    void on_saveLog();

private:
    Ui::MainWindow *ui;
    ProtocolHandler *m_protocolHandler;
    SerialPortManager *m_serialManager;
    QTimer *m_monitorTimer;
    quint64 m_bytesSent;
    quint64 m_bytesReceived;
    int m_frameCount;
    QMutex m_displayMutex;

    void setupUi();
    void setupConnections();
    void updateUIState(bool connected);
    void appendLog(const QString &text, const QString &color = "#00ff00");
    void updateStatistics(quint64 txBytes, quint64 rxBytes);
    QString getCurrentSettings() const;
    void sendPowerParam(int addr, const QString &valueStr);
    void updateProtocolFromUI();
    QString bytesToHexString(const QByteArray &data);
};

#endif // MAINWINDOW_H
