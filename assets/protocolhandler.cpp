#include "protocolhandler.h"
#include <QDebug>

ProtocolHandler::ProtocolHandler(QObject *parent)
    : QObject(parent)
{
}

ProtocolHandler::~ProtocolHandler()
{
}

QByteArray ProtocolHandler::hexStringToBytes(const QString &hexStr)
{
    QByteArray result;
    QString clean = hexStr.trimmed().replace(" ", "");
    for (int i = 0; i < clean.length(); i += 2) {
        if (i + 2 <= clean.length()) {
            bool ok;
            quint8 byte = clean.mid(i, 2).toUInt(&ok, 16);
            if (ok) {
                result.append(static_cast<char>(byte));
            }
        }
    }
    return result;
}

void ProtocolHandler::parseData(const QByteArray &data)
{
    m_buffer.append(data);

    while (m_buffer.size() >= m_minFrameLen) {
        // 查找帧头
        int headerIndex = m_buffer.indexOf(static_cast<char>(m_frameHeader));
        if (headerIndex < 0) {
            m_buffer.clear();
            break;
        }

        // 移除帧头之前的数据
        if (headerIndex > 0) {
            m_buffer.remove(0, headerIndex);
        }

        if (m_buffer.size() < m_minFrameLen) {
            break;
        }

        // 提取帧长度（根据协议，长度字节在固定位置）
        quint8 frameLen = static_cast<quint8>(m_buffer[1]);
        if (frameLen < m_minFrameLen || frameLen > 255) {
            m_buffer.remove(0, 1);
            continue;
        }

        if (m_buffer.size() < frameLen) {
            break;
        }

        QByteArray frame = m_buffer.left(frameLen);
        m_buffer.remove(0, frameLen);

        // 发送原始帧
        emit rawFrameReceived(frame);

        // 验证帧
        if (validateFrame(frame)) {
            quint8 cmd = static_cast<quint8>(frame[2]);
            quint8 status = static_cast<quint8>(frame[3]);
            QByteArray payload = frame.mid(4, frameLen - 6);
            emit frameReceived(cmd, payload, status);
        } else {
            emit parseError("校验错误");
        }
    }
}

QByteArray ProtocolHandler::buildCustomFrame(quint8 head, quint8 cmd, quint8 addr,
                                              const QByteArray &data, CheckSumType type)
{
    QByteArray frame;
    frame.append(static_cast<char>(head));                    // 帧头
    frame.append(static_cast<char>(6 + data.size()));          // 长度
    frame.append(static_cast<char>(cmd));                     // 命令
    frame.append(static_cast<char>(addr));                     // 地址
    frame.append(data);                                        // 数据
    frame.append(static_cast<char>(calculateChecksum(frame, type))); // 校验
    return frame;
}

QByteArray ProtocolHandler::buildReadConfigFrame()
{
    QByteArray frame;
    frame.append(static_cast<char>(m_frameHeader));
    frame.append(static_cast<char>(0x06));
    frame.append(static_cast<char>(0x04));  // 命令码: 读取配置
    frame.append(static_cast<char>(0x00));
    frame.append(static_cast<char>(0x00));
    frame.append(static_cast<char>(calculateChecksum(frame, m_checkSumType)));
    return frame;
}

QByteArray ProtocolHandler::buildWriteConfigFrame(quint8 addr, quint8 dataHigh, quint8 dataLow)
{
    QByteArray frame;
    frame.append(static_cast<char>(m_frameHeader));
    frame.append(static_cast<char>(0x07));
    frame.append(static_cast<char>(0x03));  // 命令码: 写入配置
    frame.append(static_cast<char>(addr));
    frame.append(static_cast<char>(dataHigh));
    frame.append(static_cast<char>(dataLow));
    frame.append(static_cast<char>(calculateChecksum(frame, m_checkSumType)));
    return frame;
}

QByteArray ProtocolHandler::buildReadStatusFrame()
{
    QByteArray frame;
    frame.append(static_cast<char>(m_frameHeader));
    frame.append(static_cast<char>(0x06));
    frame.append(static_cast<char>(0x02));  // 命令码: 读取状态
    frame.append(static_cast<char>(0x00));
    frame.append(static_cast<char>(0x00));
    frame.append(static_cast<char>(calculateChecksum(frame, m_checkSumType)));
    return frame;
}

QByteArray ProtocolHandler::buildHeartbeatFrame()
{
    QByteArray frame;
    frame.append(static_cast<char>(m_frameHeader));
    frame.append(static_cast<char>(0x05));
    frame.append(static_cast<char>(0x00));  // 命令码: 心跳
    frame.append(static_cast<char>(0x00));
    frame.append(static_cast<char>(calculateChecksum(frame, m_checkSumType)));
    return frame;
}

quint8 ProtocolHandler::calculateChecksum(const QByteArray &data, CheckSumType type)
{
    switch (type) {
        case CheckSumType::XOR: {
            quint8 result = 0;
            for (int i = 0; i < data.size(); ++i) {
                result ^= static_cast<quint8>(data[i]);
            }
            return result;
        }
        case CheckSumType::Sum: {
            quint8 result = 0;
            for (int i = 0; i < data.size(); ++i) {
                result += static_cast<quint8>(data[i]);
            }
            return result;
        }
        case CheckSumType::CRC8: {
            quint8 crc = 0;
            for (int i = 0; i < data.size(); ++i) {
                crc ^= static_cast<quint8>(data[i]);
                for (int j = 0; j < 8; ++j) {
                    if (crc & 0x80) {
                        crc = (crc << 1) ^ 0x07;
                    } else {
                        crc <<= 1;
                    }
                }
            }
            return crc;
        }
        case CheckSumType::None:
        default:
            return 0;
    }
}

bool ProtocolHandler::validateFrame(const QByteArray &frame)
{
    if (frame.size() < m_minFrameLen) {
        return false;
    }

    if (static_cast<quint8>(frame[0]) != m_frameHeader) {
        return false;
    }

    quint8 len = static_cast<quint8>(frame[1]);
    if (len != frame.size()) {
        return false;
    }

    if (m_checkSumType == CheckSumType::None) {
        return true;
    }

    QByteArray dataWithoutChecksum = frame.left(frame.size() - 1);
    quint8 calculatedChecksum = calculateChecksum(dataWithoutChecksum, m_checkSumType);
    quint8 receivedChecksum = static_cast<quint8>(frame[frame.size() - 1]);

    return (calculatedChecksum == receivedChecksum);
}
