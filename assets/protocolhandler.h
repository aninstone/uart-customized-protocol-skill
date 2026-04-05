#ifndef PROTOCOLHANDLER_H
#define PROTOCOLHANDLER_H

#include <QObject>
#include <QByteArray>
#include <QString>

class ProtocolHandler : public QObject
{
    Q_OBJECT

public:
    // 校验方式
    enum class CheckSumType {
        XOR,       // 异或校验
        Sum,       // 求和校验
        CRC8,      // CRC8
        None       // 无校验
    };

    explicit ProtocolHandler(QObject *parent = nullptr);
    ~ProtocolHandler();

    // 协议解析
    void parseData(const QByteArray &data);

    // 帧构建
    QByteArray buildReadConfigFrame();
    QByteArray buildWriteConfigFrame(quint8 addr, quint8 dataHigh, quint8 dataLow);
    QByteArray buildReadStatusFrame();
    QByteArray buildHeartbeatFrame();

    // 通用自定义帧构建
    QByteArray buildCustomFrame(quint8 head, quint8 cmd, quint8 addr,
                                const QByteArray &data, CheckSumType type = CheckSumType::XOR);

    // 协议参数设置
    void setFrameHeader(quint8 header) { m_frameHeader = header; }
    void setCheckSumType(CheckSumType type) { m_checkSumType = type; }
    void setMinFrameLen(int len) { m_minFrameLen = len; }

    quint8 getFrameHeader() const { return m_frameHeader; }
    CheckSumType getCheckSumType() const { return m_checkSumType; }

    // 从字符串解析十六进制数据
    static QByteArray hexStringToBytes(const QString &hexStr);

signals:
    void frameReceived(quint8 cmd, const QByteArray &data, quint8 status);
    void parseError(const QString &error);
    void rawFrameReceived(const QByteArray &frame);

private:
    QByteArray m_buffer;
    quint8 m_frameHeader = 0xAA;
    int m_minFrameLen = 6;
    CheckSumType m_checkSumType = CheckSumType::XOR;

    quint8 calculateChecksum(const QByteArray &data, CheckSumType type);
    bool validateFrame(const QByteArray &frame);
};

#endif // PROTOCOLHANDLER_H
