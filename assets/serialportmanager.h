#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QMutex>

// 串口配置结构体
struct SerialPortConfig {
    QString portName;
    qint32 baudRate = 115200;
    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    QSerialPort::Parity parity = QSerialPort::NoParity;
    QSerialPort::StopBits stopBits = QSerialPort::OneStop;
};

enum class SerialPortStatus {
    Disconnected,
    Connecting,
    Connected,
    Error
};

class SerialPortManager : public QObject
{
    Q_OBJECT

public:
    explicit SerialPortManager(QObject *parent = nullptr);
    ~SerialPortManager();

    // 串口操作
    bool openPort(const SerialPortConfig &config);
    void closePort();
    bool sendData(const QByteArray &data);
    bool isOpen() const;
    QStringList getAvailablePorts() const;

    // 配置
    void setReadBufferSize(qint64 size);
    void setWriteBufferSize(qint64 size);

signals:
    void statusChanged(SerialPortStatus status);
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &error);

private slots:
    void onReadyRead();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void onError(QSerialPort::SerialPortError error);
#else
    void onError(QSerialPort::SerialPortError error);
#endif

private:
    QSerialPort *m_serialPort;
    QMutex m_sendMutex;
    SerialPortConfig m_currentConfig;
    SerialPortStatus m_status;
};

#endif // SERIALPORTMANAGER_H
