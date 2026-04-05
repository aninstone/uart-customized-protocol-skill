#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "protocolhandler.h"
#include "serialportmanager.h"

#include <QMessageBox>
#include <QDateTime>
#include <QFileDialog>
#include <QSerialPortInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_protocolHandler(new ProtocolHandler(this))
    , m_serialManager(new SerialPortManager(this))
    , m_bytesSent(0)
    , m_bytesReceived(0)
    , m_frameCount(0)
    , m_monitorTimer(new QTimer(this))
{
    ui->setupUi(this);
    resize(1200, 800);
    setMinimumSize(1000, 700);
    setupUi();
    setupConnections();
    on_refreshPorts();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    // 整体深色主题
    setStyleSheet(
        "QWidget { background-color: #2b2b2b; color: #e0e0e0; font-family: 'Microsoft YaHei', 'Segoe UI', sans-serif; }"
        "QLabel { color: #e0e0e0; }"
        "QGroupBox { border: 2px solid #444; border-radius: 5px; margin-top: 10px; font-weight: bold; color: #4a9eff; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 5px; }"
        "QPushButton { background-color: #3a3a3a; border: 1px solid #555; border-radius: 4px; padding: 6px 16px; color: #e0e0e0; min-width: 70px; }"
        "QPushButton:hover { background-color: #4a4a4a; border-color: #4a9eff; }"
        "QPushButton:pressed { background-color: #333; }"
        "QComboBox { background-color: #3a3a3a; border: 1px solid #555; border-radius: 4px; padding: 4px 8px; color: #e0e0e0; }"
        "QComboBox:hover { border-color: #4a9eff; }"
        "QLineEdit { background-color: #333; border: 1px solid #555; border-radius: 4px; padding: 4px 8px; color: #00ff00; selection-background-color: #4a9eff; }"
        "QTextEdit { background-color: #1e1e1e; color: #00ff00; border: 2px solid #2c3e50; border-radius: 5px; }"
        "QTableWidget { background-color: #2b2b2b; alternate-background-color: #333; gridline-color: #444; color: #e0e0e0; }"
        "QHeaderView::section { background-color: #3a3a3a; color: #e0e0e0; padding: 5px; border: 1px solid #555; }"
        "QTabWidget::pane { border: 2px solid #444; background-color: #2b2b2b; }"
        "QTabBar::tab { background-color: #2b2b2b; color: #aaa; padding: 8px 20px; border: 1px solid #444; }"
        "QTabBar::tab:selected { background-color: #3a3a3a; color: #4a9eff; border-bottom: 2px solid #4a9eff; }"
        "QTabBar::tab:hover { background-color: #333; }"
    );

    // 波特率
    ui->comboBoxBaud->addItems({"9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600"});
    ui->comboBoxBaud->setCurrentIndex(4);

    // 数据位
    ui->comboBoxDataBits->addItems({"5", "6", "7", "8"});
    ui->comboBoxDataBits->setCurrentIndex(3);

    // 校验位
    ui->comboBoxParity->addItems({"None", "Odd", "Even", "Mark", "Space"});
    ui->comboBoxParity->setCurrentIndex(0);

    // 停止位
    ui->comboBoxStopBits->addItems({"1", "1.5", "2"});
    ui->comboBoxStopBits->setCurrentIndex(0);

    // 协议配置 - 校验方式
    ui->comboBoxCheckSum->addItems({"XOR(异或)", "Sum(求和)", "CRC8", "None(无)"});

    // 协议配置 - 默认值
    ui->lineEdit_FrameHead->setText("AA");
    ui->lineEdit_DeviceAddr->setText("01");
    ui->lineEdit_CmdRead->setText("04");
    ui->lineEdit_CmdWrite->setText("03");
    ui->lineEdit_CmdStatus->setText("02");

    // 初始化监控表格
    ui->tableWidget_Monitor->setRowCount(10);
    QStringList labels = {"输出电压", "输出电流", "输出功率", "输入电压", "模块温度", "工作状态", "告警状态", "风扇转速", "运行时间", "版本信息"};
    QStringList units = {"V", "A", "W", "V", "℃", "-", "-", "%", "h", "-"};
    for (int i = 0; i < 10; ++i) {
        ui->tableWidget_Monitor->setItem(i, 0, new QTableWidgetItem(labels[i]));
        ui->tableWidget_Monitor->setItem(i, 1, new QTableWidgetItem("0"));
        ui->tableWidget_Monitor->setItem(i, 2, new QTableWidgetItem(units[i]));
        ui->tableWidget_Monitor->setItem(i, 3, new QTableWidgetItem("正常"));
    }
    ui->tableWidget_Monitor->setColumnWidth(0, 150);
    ui->tableWidget_Monitor->setColumnWidth(1, 120);
    ui->tableWidget_Monitor->setColumnWidth(2, 80);
    ui->tableWidget_Monitor->setColumnWidth(3, 100);

    // 禁用手动控制按钮（初始状态）
    ui->btnRead->setEnabled(false);
    ui->btnWrite->setEnabled(false);
    ui->btnHexSend->setEnabled(false);

    // 监控定时器连接
    connect(m_monitorTimer, &QTimer::timeout, this, &MainWindow::on_monitorRefresh);
}

void MainWindow::setupConnections()
{
    // 串口操作
    connect(ui->btnRefresh, &QPushButton::clicked, this, &MainWindow::on_refreshPorts);
    connect(ui->btnOpenClose, &QPushButton::clicked, this, &MainWindow::on_openClosePort);

    // 手动控制
    connect(ui->btnRead, &QPushButton::clicked, this, &MainWindow::on_readSend);
    connect(ui->btnWrite, &QPushButton::clicked, this, &MainWindow::on_writeSend);
    connect(ui->btnHexSend, &QPushButton::clicked, this, &MainWindow::on_hexSend);
    connect(ui->btnBuildFrame, &QPushButton::clicked, this, &MainWindow::on_buildFrame);

    // 协议配置变化
    connect(ui->lineEdit_FrameHead, &QLineEdit::editingFinished, this, &MainWindow::on_protocolConfigChanged);
    connect(ui->comboBoxCheckSum, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::on_protocolConfigChanged);

    // 电源参数 - 写入按钮
    connect(ui->btnWrite_P1, &QPushButton::clicked, this, &MainWindow::on_powerWriteP1);
    connect(ui->btnWrite_P2, &QPushButton::clicked, this, &MainWindow::on_powerWriteP2);
    connect(ui->btnWrite_P3, &QPushButton::clicked, this, &MainWindow::on_powerWriteP3);
    connect(ui->btnWrite_P4, &QPushButton::clicked, this, &MainWindow::on_powerWriteP4);
    connect(ui->btnWrite_P5, &QPushButton::clicked, this, &MainWindow::on_powerWriteP5);
    connect(ui->btnWrite_P6, &QPushButton::clicked, this, &MainWindow::on_powerWriteP6);
    connect(ui->btnWrite_P7, &QPushButton::clicked, this, &MainWindow::on_powerWriteP7);
    connect(ui->btnWrite_P8, &QPushButton::clicked, this, &MainWindow::on_powerWriteP8);
    connect(ui->btnWrite_P9, &QPushButton::clicked, this, &MainWindow::on_powerWriteP9);
    connect(ui->btnWrite_P10, &QPushButton::clicked, this, &MainWindow::on_powerWriteP10);

    // 电源参数 - 全部操作
    connect(ui->btnReadAllPower, &QPushButton::clicked, this, &MainWindow::on_powerReadAll);
    connect(ui->btnWriteAllPower, &QPushButton::clicked, this, &MainWindow::on_powerWriteAll);

    // 运行监控
    connect(ui->btnMonitorStart, &QPushButton::clicked, this, &MainWindow::on_monitorStart);
    connect(ui->btnMonitorStop, &QPushButton::clicked, this, &MainWindow::on_monitorStop);
    connect(ui->btnMonitorRefresh, &QPushButton::clicked, this, &MainWindow::on_monitorRefresh);
    connect(ui->btnMonitorStart2, &QPushButton::clicked, this, &MainWindow::on_monitorStart);
    connect(ui->btnMonitorStop2, &QPushButton::clicked, this, &MainWindow::on_monitorStop);
    connect(ui->btnMonitorClear, &QPushButton::clicked, [this]() {
        for (int i = 0; i < 10; ++i) {
            ui->tableWidget_Monitor->item(i, 1)->setText("0");
            ui->tableWidget_Monitor->item(i, 3)->setText("正常");
        }
    });

    // 显示控制
    connect(ui->btnClearLog, &QPushButton::clicked, this, &MainWindow::on_clearLog);
    connect(ui->btnSaveLog, &QPushButton::clicked, this, &MainWindow::on_saveLog);

    // 协议处理器信号
    connect(m_protocolHandler, &ProtocolHandler::frameReceived, this, &MainWindow::on_dataReceived);
    connect(m_protocolHandler, &ProtocolHandler::rawFrameReceived, this, &MainWindow::on_rawFrameReceived);
    connect(m_protocolHandler, &ProtocolHandler::parseError, this, &MainWindow::on_errorOccurred);

    // 串口管理器信号
    connect(m_serialManager, &SerialPortManager::statusChanged, this, &MainWindow::on_connectionStateChanged);
    connect(m_serialManager, &SerialPortManager::dataReceived, this, &MainWindow::on_serialDataReceived);
}

void MainWindow::updateProtocolFromUI()
{
    bool ok;
    quint8 head = ui->lineEdit_FrameHead->text().toUInt(&ok, 16);
    if (!ok) head = 0xAA;
    m_protocolHandler->setFrameHeader(head);

    ProtocolHandler::CheckSumType type;
    switch (ui->comboBoxCheckSum->currentIndex()) {
        case 1: type = ProtocolHandler::CheckSumType::Sum; break;
        case 2: type = ProtocolHandler::CheckSumType::CRC8; break;
        case 3: type = ProtocolHandler::CheckSumType::None; break;
        default: type = ProtocolHandler::CheckSumType::XOR; break;
    }
    m_protocolHandler->setCheckSumType(type);
}

void MainWindow::on_protocolConfigChanged()
{
    updateProtocolFromUI();
}

QString MainWindow::bytesToHexString(const QByteArray &data)
{
    QString hex;
    for (int i = 0; i < data.size(); i++) {
        hex += QString("%1 ").arg(static_cast<quint8>(data[i]), 2, 16, QChar('0')).toUpper();
    }
    return hex.trimmed();
}

void MainWindow::on_refreshPorts()
{
    ui->comboBoxPort->clear();
    QStringList ports = m_serialManager->getAvailablePorts();

    if (ports.isEmpty()) {
        ui->comboBoxPort->addItem("无可用串口");
        ui->comboBoxPort->setEnabled(false);
    } else {
        ui->comboBoxPort->setEnabled(true);
        for (const QString &port : ports) {
            ui->comboBoxPort->addItem(port);
        }
    }
}

void MainWindow::on_openClosePort()
{
    if (m_serialManager->isOpen()) {
        m_serialManager->closePort();
        m_monitorTimer->stop();
    } else {
        QString portName = ui->comboBoxPort->currentText();
        if (portName.isEmpty() || portName == "无可用串口") {
            QMessageBox::warning(this, "警告", "没有可用的串口!");
            return;
        }

        SerialPortConfig config;
        config.portName = portName;

        bool ok;
        config.baudRate = ui->comboBoxBaud->currentText().toInt(&ok);
        if (!ok) config.baudRate = 115200;

        switch (ui->comboBoxDataBits->currentIndex()) {
            case 0: config.dataBits = QSerialPort::Data5; break;
            case 1: config.dataBits = QSerialPort::Data6; break;
            case 2: config.dataBits = QSerialPort::Data7; break;
            default: config.dataBits = QSerialPort::Data8; break;
        }

        switch (ui->comboBoxParity->currentIndex()) {
            case 1: config.parity = QSerialPort::OddParity; break;
            case 2: config.parity = QSerialPort::EvenParity; break;
            case 3: config.parity = QSerialPort::MarkParity; break;
            case 4: config.parity = QSerialPort::SpaceParity; break;
            default: config.parity = QSerialPort::NoParity; break;
        }

        switch (ui->comboBoxStopBits->currentIndex()) {
            case 1: config.stopBits = QSerialPort::OneAndHalfStop; break;
            case 2: config.stopBits = QSerialPort::TwoStop; break;
            default: config.stopBits = QSerialPort::OneStop; break;
        }

        if (!m_serialManager->openPort(config)) {
            QMessageBox::critical(this, "错误", "无法打开串口!");
        }
    }
}

void MainWindow::on_serialDataReceived(const QByteArray &data)
{
    m_bytesReceived += data.size();
    updateStatistics(m_bytesSent, m_bytesReceived);

    QString hexStr = bytesToHexString(data);
    QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    appendLog(QString("[%1] RX: %2").arg(timeStr).arg(hexStr), "#27ae60");

    m_protocolHandler->parseData(data);
}

void MainWindow::on_buildFrame()
{
    updateProtocolFromUI();

    bool ok;
    quint8 head = ui->lineEdit_FrameHead->text().toUInt(&ok, 16);
    quint8 addr = ui->lineEdit_DeviceAddr->text().toUInt(&ok, 16);
    quint8 cmd = ui->lineEdit_CustomCmd->text().toUInt(&ok, 16);
    QByteArray data = ProtocolHandler::hexStringToBytes(ui->lineEdit_CustomData->text());

    ProtocolHandler::CheckSumType type;
    switch (ui->comboBoxCheckSum->currentIndex()) {
        case 1: type = ProtocolHandler::CheckSumType::Sum; break;
        case 2: type = ProtocolHandler::CheckSumType::CRC8; break;
        case 3: type = ProtocolHandler::CheckSumType::None; break;
        default: type = ProtocolHandler::CheckSumType::XOR; break;
    }

    QByteArray frame = m_protocolHandler->buildCustomFrame(head, cmd, addr, data, type);
    ui->lineEdit_CustomResult->setText(bytesToHexString(frame));
    appendLog(QString("组帧完成: %1").arg(bytesToHexString(frame)), "#4a9eff");
}

void MainWindow::on_readSend()
{
    if (!m_serialManager->isOpen()) {
        QMessageBox::warning(this, "警告", "请先打开串口!");
        return;
    }

    updateProtocolFromUI();

    bool ok;
    quint8 addr = ui->lineEdit_DeviceAddr->text().toUInt(&ok, 16);
    quint8 cmd = ui->lineEdit_CmdRead->text().toUInt(&ok, 16);

    QByteArray data = ProtocolHandler::hexStringToBytes(ui->lineEdit_Data->text());

    ProtocolHandler::CheckSumType type;
    switch (ui->comboBoxCheckSum->currentIndex()) {
        case 1: type = ProtocolHandler::CheckSumType::Sum; break;
        case 2: type = ProtocolHandler::CheckSumType::CRC8; break;
        case 3: type = ProtocolHandler::CheckSumType::None; break;
        default: type = ProtocolHandler::CheckSumType::XOR; break;
    }

    QByteArray frame = m_protocolHandler->buildCustomFrame(
        m_protocolHandler->getFrameHeader(), cmd, addr, data, type);

    if (m_serialManager->sendData(frame)) {
        m_bytesSent += frame.size();
        updateStatistics(m_bytesSent, m_bytesReceived);
        appendLog(QString("[TX-读] %1").arg(bytesToHexString(frame)), "#3498db");
    }
}

void MainWindow::on_writeSend()
{
    if (!m_serialManager->isOpen()) {
        QMessageBox::warning(this, "警告", "请先打开串口!");
        return;
    }

    updateProtocolFromUI();

    bool ok;
    quint8 addr = ui->lineEdit_DeviceAddr->text().toUInt(&ok, 16);
    quint8 cmd = ui->lineEdit_CmdWrite->text().toUInt(&ok, 16);

    QByteArray data = ProtocolHandler::hexStringToBytes(ui->lineEdit_Data->text());

    ProtocolHandler::CheckSumType type;
    switch (ui->comboBoxCheckSum->currentIndex()) {
        case 1: type = ProtocolHandler::CheckSumType::Sum; break;
        case 2: type = ProtocolHandler::CheckSumType::CRC8; break;
        case 3: type = ProtocolHandler::CheckSumType::None; break;
        default: type = ProtocolHandler::CheckSumType::XOR; break;
    }

    QByteArray frame = m_protocolHandler->buildCustomFrame(
        m_protocolHandler->getFrameHeader(), cmd, addr, data, type);

    if (m_serialManager->sendData(frame)) {
        m_bytesSent += frame.size();
        updateStatistics(m_bytesSent, m_bytesReceived);
        appendLog(QString("[TX-写] %1").arg(bytesToHexString(frame)), "#e67e22");
    }
}

void MainWindow::on_hexSend()
{
    if (!m_serialManager->isOpen()) {
        QMessageBox::warning(this, "警告", "请先打开串口!");
        return;
    }

    QString hexStr = ui->lineEdit_HexRaw->text().trimmed().replace(" ", "");
    QByteArray frame = ProtocolHandler::hexStringToBytes(hexStr);

    if (frame.isEmpty()) {
        QMessageBox::warning(this, "警告", "无效的十六进制数据!");
        return;
    }

    if (m_serialManager->sendData(frame)) {
        m_bytesSent += frame.size();
        updateStatistics(m_bytesSent, m_bytesReceived);
        appendLog(QString("[TX-HEX] %1").arg(bytesToHexString(frame)), "#9b59b6");
    }
}

// 电源参数页面
void MainWindow::on_powerWriteP1() { sendPowerParam(1, ui->lineEdit_P1->text()); }
void MainWindow::on_powerWriteP2() { sendPowerParam(2, ui->lineEdit_P2->text()); }
void MainWindow::on_powerWriteP3() { sendPowerParam(3, ui->lineEdit_P3->text()); }
void MainWindow::on_powerWriteP4() { sendPowerParam(4, ui->lineEdit_P4->text()); }
void MainWindow::on_powerWriteP5() { sendPowerParam(5, ui->lineEdit_P5->text()); }
void MainWindow::on_powerWriteP6() { sendPowerParam(6, ui->lineEdit_P6->text()); }
void MainWindow::on_powerWriteP7() { sendPowerParam(7, ui->lineEdit_P7->text()); }
void MainWindow::on_powerWriteP8() { sendPowerParam(8, ui->lineEdit_P8->text()); }
void MainWindow::on_powerWriteP9() { sendPowerParam(9, ui->lineEdit_P9->text()); }
void MainWindow::on_powerWriteP10() { sendPowerParam(10, ui->lineEdit_P10->text()); }

void MainWindow::sendPowerParam(int addr, const QString &valueStr)
{
    if (!m_serialManager->isOpen()) {
        QMessageBox::warning(this, "警告", "请先打开串口!");
        return;
    }
    bool ok;
    quint16 value = valueStr.toUShort(&ok);
    if (!ok) value = 0;

    QByteArray frame = m_protocolHandler->buildWriteConfigFrame(static_cast<quint8>(addr),
        static_cast<quint8>(value >> 8), static_cast<quint8>(value & 0xFF));
    if (m_serialManager->sendData(frame)) {
        m_bytesSent += frame.size();
        updateStatistics(m_bytesSent, m_bytesReceived);
        appendLog(QString("[TX] 写入参数%1: %2").arg(addr).arg(value), "#27ae60");
    }
}

void MainWindow::on_powerReadAll()
{
    if (!m_serialManager->isOpen()) {
        QMessageBox::warning(this, "警告", "请先打开串口!");
        return;
    }
    updateProtocolFromUI();
    QByteArray frame = m_protocolHandler->buildReadConfigFrame();
    if (m_serialManager->sendData(frame)) {
        m_bytesSent += frame.size();
        updateStatistics(m_bytesSent, m_bytesReceived);
        appendLog("[TX] 读取全部电源参数", "#3498db");
    }
}

void MainWindow::on_powerWriteAll()
{
    if (!m_serialManager->isOpen()) {
        QMessageBox::warning(this, "警告", "请先打开串口!");
        return;
    }

    updateProtocolFromUI();

    QList<QLineEdit*> edits = {
        ui->lineEdit_P1, ui->lineEdit_P2, ui->lineEdit_P3, ui->lineEdit_P4, ui->lineEdit_P5,
        ui->lineEdit_P6, ui->lineEdit_P7, ui->lineEdit_P8, ui->lineEdit_P9, ui->lineEdit_P10
    };

    for (int i = 0; i < edits.size(); i++) {
        bool ok;
        quint16 value = edits[i]->text().toUShort(&ok);
        if (!ok) value = 0;

        QByteArray frame = m_protocolHandler->buildWriteConfigFrame(static_cast<quint8>(i + 1),
            static_cast<quint8>(value >> 8), static_cast<quint8>(value & 0xFF));
        m_serialManager->sendData(frame);
        m_bytesSent += frame.size();
    }

    updateStatistics(m_bytesSent, m_bytesReceived);
    appendLog("[TX] 写入全部电源参数", "#e67e22");
}

// 运行监控
void MainWindow::on_monitorStart()
{
    if (!m_serialManager->isOpen()) {
        QMessageBox::warning(this, "警告", "请先打开串口!");
        return;
    }
    updateProtocolFromUI();
    m_monitorTimer->start(500);
    appendLog("[TX] 启动定时监控", "#27ae60");
}

void MainWindow::on_monitorStop()
{
    m_monitorTimer->stop();
    appendLog("[TX] 停止定时监控", "#e74c3c");
}

void MainWindow::on_monitorRefresh()
{
    if (!m_serialManager->isOpen()) {
        m_monitorTimer->stop();
        return;
    }
    updateProtocolFromUI();
    QByteArray frame = m_protocolHandler->buildReadStatusFrame();
    m_serialManager->sendData(frame);
}

// 数据处理
void MainWindow::on_dataReceived(quint8 cmd, const QByteArray &data, quint8 status)
{
    m_frameCount++;
    QString statusStr = (status == 0) ? "成功" : "失败";
    appendLog(QString("帧解析: CMD=%1 [%2] 数据: %3").arg(cmd, 2, 16)
        .arg(statusStr).arg(bytesToHexString(data)), "#f39c12");

    // 更新UI显示
    if (data.size() >= 1) {
        ui->lineEdit_R1->setText(QString::number(static_cast<quint8>(data[0])));
        if (ui->tableWidget_Monitor->item(0, 1)) ui->tableWidget_Monitor->item(0, 1)->setText(QString::number(static_cast<quint8>(data[0])));
    }
    if (data.size() >= 2) {
        ui->lineEdit_R2->setText(QString::number(static_cast<quint8>(data[1])));
        if (ui->tableWidget_Monitor->item(1, 1)) ui->tableWidget_Monitor->item(1, 1)->setText(QString::number(static_cast<quint8>(data[1])));
    }
    if (data.size() >= 3) {
        ui->lineEdit_R3->setText(QString::number(static_cast<quint8>(data[2])));
        if (ui->tableWidget_Monitor->item(2, 1)) ui->tableWidget_Monitor->item(2, 1)->setText(QString::number(static_cast<quint8>(data[2])));
    }
    if (data.size() >= 4) {
        ui->lineEdit_R4->setText(QString::number(static_cast<quint8>(data[3])));
        if (ui->tableWidget_Monitor->item(3, 1)) ui->tableWidget_Monitor->item(3, 1)->setText(QString::number(static_cast<quint8>(data[3])));
    }
    if (data.size() >= 5) {
        ui->lineEdit_R5->setText(QString::number(static_cast<quint8>(data[4])));
        if (ui->tableWidget_Monitor->item(4, 1)) ui->tableWidget_Monitor->item(4, 1)->setText(QString::number(static_cast<quint8>(data[4])));
    }
    if (data.size() >= 6) {
        ui->lineEdit_R6->setText(QString::number(static_cast<quint8>(data[5])));
        if (ui->tableWidget_Monitor->item(5, 1)) ui->tableWidget_Monitor->item(5, 1)->setText(QString::number(static_cast<quint8>(data[5])));
    }
    if (data.size() >= 7) {
        ui->lineEdit_R7->setText(QString::number(static_cast<quint8>(data[6])));
        if (ui->tableWidget_Monitor->item(6, 1)) ui->tableWidget_Monitor->item(6, 1)->setText(QString::number(static_cast<quint8>(data[6])));
    }
    if (data.size() >= 8) {
        ui->lineEdit_R8->setText(QString::number(static_cast<quint8>(data[7])));
        if (ui->tableWidget_Monitor->item(7, 1)) ui->tableWidget_Monitor->item(7, 1)->setText(QString::number(static_cast<quint8>(data[7])));
    }
    if (data.size() >= 9) {
        ui->lineEdit_R9->setText(QString::number(static_cast<quint8>(data[8])));
        if (ui->tableWidget_Monitor->item(8, 1)) ui->tableWidget_Monitor->item(8, 1)->setText(QString::number(static_cast<quint8>(data[8])));
    }
    if (data.size() >= 10) {
        ui->lineEdit_R10->setText(QString::number(static_cast<quint8>(data[9])));
        if (ui->tableWidget_Monitor->item(9, 1)) ui->tableWidget_Monitor->item(9, 1)->setText(QString::number(static_cast<quint8>(data[9])));
    }
}

void MainWindow::on_rawFrameReceived(const QByteArray &frame)
{
    // 原始帧显示（用于调试）
}

void MainWindow::on_errorOccurred(const QString &error)
{
    appendLog(QString("[错误] %1").arg(error), "#e74c3c");
}

void MainWindow::on_connectionStateChanged(SerialPortStatus status)
{
    bool connected = (status == SerialPortStatus::Connected);
    updateUIState(connected);

    if (connected) {
        appendLog("串口已打开: " + getCurrentSettings(), "#27ae60");
    } else {
        appendLog("串口已关闭", "#e74c3c");
        m_monitorTimer->stop();
    }
}

void MainWindow::on_clearLog()
{
    ui->textEditLog->clear();
    m_bytesSent = 0;
    m_bytesReceived = 0;
    m_frameCount = 0;
    updateStatistics(0, 0);
}

void MainWindow::on_saveLog()
{
    QString fileName = QFileDialog::getSaveFileName(this, "保存日志",
        QString("digitaluart_log_%1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "文本文件 (*.txt);;所有文件 (*.*)");

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(ui->textEditLog->toPlainText().toUtf8());
            file.close();
            QMessageBox::information(this, "成功", "日志已保存!");
        } else {
            QMessageBox::critical(this, "错误", "无法保存文件!");
        }
    }
}

void MainWindow::updateUIState(bool connected)
{
    if (connected) {
        ui->btnOpenClose->setText("关闭串口");
        ui->btnOpenClose->setStyleSheet("background-color: #e74c3c; color: white;");
        ui->comboBoxPort->setEnabled(false);
        ui->comboBoxBaud->setEnabled(false);
        ui->comboBoxDataBits->setEnabled(false);
        ui->comboBoxParity->setEnabled(false);
        ui->comboBoxStopBits->setEnabled(false);
        ui->btnRefresh->setEnabled(false);
        ui->btnRead->setEnabled(true);
        ui->btnWrite->setEnabled(true);
        ui->btnHexSend->setEnabled(true);
        ui->labelStatus->setText("● 已连接");
        ui->labelStatus->setStyleSheet("color: #27ae60; font-weight: bold; font-size: 12px;");
    } else {
        ui->btnOpenClose->setText("打开串口");
        ui->btnOpenClose->setStyleSheet("");
        ui->comboBoxPort->setEnabled(true);
        ui->comboBoxBaud->setEnabled(true);
        ui->comboBoxDataBits->setEnabled(true);
        ui->comboBoxParity->setEnabled(true);
        ui->comboBoxStopBits->setEnabled(true);
        ui->btnRefresh->setEnabled(true);
        ui->btnRead->setEnabled(false);
        ui->btnWrite->setEnabled(false);
        ui->btnHexSend->setEnabled(false);
        ui->labelStatus->setText("● 未连接");
        ui->labelStatus->setStyleSheet("color: #e74c3c; font-weight: bold; font-size: 12px;");
    }
}

void MainWindow::appendLog(const QString &text, const QString &color)
{
    QMutexLocker locker(&m_displayMutex);

    QTextCursor cursor(ui->textEditLog->textCursor());
    cursor.movePosition(QTextCursor::End);

    QTextCharFormat format;
    format.setForeground(QColor(color));
    cursor.setCharFormat(format);
    cursor.insertText(text + "\n");

    ui->textEditLog->setTextCursor(cursor);
    ui->textEditLog->ensureCursorVisible();
}

void MainWindow::updateStatistics(quint64 txBytes, quint64 rxBytes)
{
    ui->labelTxBytes->setText(QString::number(txBytes));
    ui->labelRxBytes->setText(QString::number(rxBytes));
    ui->labelFrameCount->setText(QString::number(m_frameCount));
}

QString MainWindow::getCurrentSettings() const
{
    return QString("%1, %2, %3-%4-%5")
        .arg(ui->comboBoxPort->currentText())
        .arg(ui->comboBoxBaud->currentText())
        .arg(ui->comboBoxDataBits->currentText())
        .arg(ui->comboBoxParity->currentText().left(1))
        .arg(ui->comboBoxStopBits->currentText());
}
