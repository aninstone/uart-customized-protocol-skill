#include "serialportmanager.h"
#include <QDebug>

SerialPortManager::SerialPortManager(QObject *parent)
    : QObject(parent)
    , m_serialPort(new QSerialPort(this))
    , m_status(SerialPortStatus::Disconnected)
{
    // 连接信号槽
    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialPortManager::onReadyRead);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &SerialPortManager::onError);
#else
    connect(m_serialPort, QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::error),
            this, &SerialPortManager::onError);
#endif
}

SerialPortManager::~SerialPortManager()
{
    closePort();
}

bool SerialPortManager::openPort(const SerialPortConfig &config)
{
    if (m_serialPort->isOpen()) {
        closePort();
    }

    m_currentConfig = config;
    m_serialPort->setPortName(config.portName);

    if (!m_serialPort->open(QIODevice::ReadWrite)) {
        emit errorOccurred(QString("无法打开串口: %1").arg(m_serialPort->errorString()));
        m_status = SerialPortStatus::Error;
        emit statusChanged(m_status);
        return false;
    }

    // 配置串口参数
    m_serialPort->setBaudRate(config.baudRate);
    m_serialPort->setDataBits(config.dataBits);
    m_serialPort->setParity(config.parity);
    m_serialPort->setStopBits(config.stopBits);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    m_status = SerialPortStatus::Connected;
    emit statusChanged(m_status);
    return true;
}

void SerialPortManager::closePort()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }
    m_status = SerialPortStatus::Disconnected;
    emit statusChanged(m_status);
}

bool SerialPortManager::sendData(const QByteArray &data)
{
    if (!m_serialPort->isOpen()) {
        return false;
    }

    QMutexLocker locker(&m_sendMutex);
    qint64 written = m_serialPort->write(data);
    m_serialPort->flush();
    return (written == data.size());
}

bool SerialPortManager::isOpen() const
{
    return m_serialPort->isOpen();
}

QStringList SerialPortManager::getAvailablePorts() const
{
    QStringList ports;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        ports.append(info.portName());
    }
    return ports;
}

void SerialPortManager::setReadBufferSize(qint64 size)
{
    m_serialPort->setReadBufferSize(size);
}

void SerialPortManager::setWriteBufferSize(qint64 size)
{
    Q_UNUSED(size);
    // QSerialPort 不支持单独设置写入缓冲区大小
}

void SerialPortManager::onReadyRead()
{
    QByteArray data = m_serialPort->readAll();
    if (!data.isEmpty()) {
        emit dataReceived(data);
    }
}

void SerialPortManager::onError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError && error != QSerialPort::TimeoutError) {
        QString errorString = m_serialPort->errorString();
        emit errorOccurred(errorString);

        if (error == QSerialPort::ResourceError || error == QSerialPort::DeviceNotFoundError) {
            closePort();
        }
    }
}
