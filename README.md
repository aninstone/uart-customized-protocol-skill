# UART Customized Protocol Generator Skill

一个用于快速创建 **自定义 UART 协议** 上位机应用的 CodeBuddy Skill。

**模板仓库**: [uart-customized-protocol-skill](https://github.com/aninstone/uart-customized-protocol-skill)

## 功能特点

- ✅ 基于 `digitaluartcontrol-skill` 模板，已验证可运行
- ✅ **自定义协议**：帧头、设备地址、命令码、校验方式全部可配置
- ✅ **多种校验算法**：XOR(异或)、Sum(求和)、CRC8、None(无)
- ✅ **三页面 UI**：手动控制、电源参数、运行监控
- ✅ **深色主题**：专业工业风格 (#2b2b2b 背景, #00ff00 日志)
- ✅ **Qt 5/6 兼容**：自动处理信号名称差异
- ✅ 串口自动检测与配置
- ✅ 实时收发统计与日志记录

## 协议帧格式

```
[帧头 1B][长度 1B][命令码 1B][地址 1B][数据 N][校验和 1B]
```

### 支持的命令

| 命令码 | 功能 | 说明 |
|--------|------|------|
| 0x02 | 状态命令 | 读取设备状态 |
| 0x03 | 写命令 | 写入配置参数 |
| 0x04 | 读命令 | 读取配置参数 |

### 支持的校验方式

| 方式 | 说明 |
|------|------|
| XOR | 异或校验（默认） |
| Sum | 求和校验 |
| CRC8 | CRC8 (多项式 0x07) |
| None | 无校验 |

## 项目结构

```
digitaluartcontrol/
├── digitaluartcontrol.pro    # Qt 项目文件
├── main.cpp                  # 程序入口
├── mainwindow.h/cpp          # 主窗口（UI + 逻辑）
├── mainwindow.ui             # UI 界面文件 (1100+ 行)
├── protocolhandler.h/cpp     # 协议解析与组帧
├── serialportmanager.h/cpp   # 串口管理 (Qt5/6兼容)
├── PROTOCOL.md               # 协议详细说明
└── README.md
```

## 界面预览

### 手动控制页面

```
┌──────────────────────────────────────────────────────────────────┐
│ [串口设置]              [协议配置]                [自定义帧]     │
│  串口: [COM7    ▼]     帧头: [AA    ]          命令码: [04  ]  │
│  波特率: [115200 ▼]    设备地址: [01  ]         数据: [00 01]  │
│  数据位: [8      ▼]    读命令: [04  ]          组帧: [AA...]  │
│  校验位: [None   ▼]    写命令: [03  ]    [组帧] [读] [写]     │
│  停止位: [1      ▼]    状态命令: [02]                          │
│  [打开串口]            校验: [XOR ▼]        [直接发送HEX]      │
├──────────────────────────────────────────────────────────────────┤
│ [通信统计] TX: 0 字节  RX: 0 字节  帧数: 0  ● 未连接         │
├──────────────────────────────────────────────────────────────────┤
│ [通信日志]                                                     │
│ 10:23:45.123 [TX-读] AA 06 04 01 00 00 B1                      │
│ 10:23:45.234 [RX] AA 07 04 00 01 02 03 04 E3                  │
│                              [清空日志] [保存日志]                │
└──────────────────────────────────────────────────────────────────┘
```

### 电源参数页面

```
┌──────────────────────────────────────────────────────────────────┐
│ 【设置参数】                                          【实际值】  │
│ 输出电压 (V): [240    ] [写入]      实际输出电压 (V): [0    ]  │
│ 输出电流 (A): [5.0    ] [写入]      实际输出电流 (A): [0    ]  │
│ 输出功率 (W): [1200   ] [写入]      实际输出功率 (W): [0    ]  │
│ 过压保护 (V): [260    ] [写入]      输入电压 (V):      [0    ]  │
│ 过流保护 (A): [6.0    ] [写入]      模块温度 (℃):     [0    ]  │
│ 过温保护 (℃): [85     ] [写入]      工作状态:          [0    ]  │
│ 输出使能:      [1      ] [写入]      告警状态:          [0    ]  │
│ 远程控制:      [1      ] [写入]      风扇转速 (%):     [0    ]  │
│ 预留参数:      [0      ] [写入]      运行时间 (h):     [0    ]  │
│ 预留参数2:     [0      ] [写入]      版本信息:         [0    ]  │
│              [读取全部] [写入全部] [启动监控] [停止监控]          │
└──────────────────────────────────────────────────────────────────┘
```

### 运行监控页面

```
┌──────────────────────────────────────────────────────────────────┐
│ 参数名称      │ 当前值    │ 单位 │ 状态                          │
├──────────────┼───────────┼──────┼───────────────────────────────┤
│ 输出电压      │ 0         │ V    │ 正常                          │
│ 输出电流      │ 0         │ A    │ 正常                          │
│ 输出功率      │ 0         │ W    │ 正常                          │
│ ...          │ ...       │ ...  │ ...                           │
├──────────────────────────────────────────────────────────────────┤
│        [刷新数据] [清空数据]          [启动定时] [停止定时]        │
└──────────────────────────────────────────────────────────────────┘
```

## 使用方法

### 方式一：在 CodeBuddy 中直接使用

```
用 https://github.com/aninstone/uart-customized-protocol-skill 作为模板，
在 D:/WorkPRJ/QT PRJ 创建一个叫 my_uart 的上位机工程。
```

### 方式二：克隆到本地

```bash
git clone https://github.com/aninstone/uart-customized-protocol-skill.git
cd uart-customized-protocol
mv assets/* ./
rm -rf assets/
mv digitaluartcontrol.pro my_project.pro
```

复制文件到你的 Qt 项目即可使用。

## 深色主题颜色

| 用途 | 颜色代码 | 说明 |
|------|----------|------|
| 主背景 | #2b2b2b | 深灰色背景 |
| 强调色 | #4a9eff | Tab选中、按钮悬停 |
| 日志文字 | #00ff00 | 绿色高亮日志 |
| 输入框背景 | #333 | TextEdit/LineEdit |
| 日志区背景 | #1e1e1e | 日志显示区 |
| 成功色 | #27ae60 | TX成功、已连接 |
| 警告色 | #e67e22 | TX写入、告警 |
| 错误色 | #e74c3c | 错误、断开连接 |

---

## ⚠️ 重要注意事项

### 1. Qt SerialPort 库

首次使用前必须安装 Qt SerialPort 库：

1. 打开 **Qt Maintenance Tool** (`C:/Qt/MaintenanceTool.exe`)
2. 添加或移除组件
3. 展开 **Qt 6.x.x** → **库**
4. 勾选 **Qt Serial Port**

### 2. 头文件 include 完整性

在 `.h` 文件中出现的任何 Qt 类型，都必须添加对应的 `#include`：

```cpp
// ✅ 正确：所有用到的类型都要 include
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QTextEdit>
#include <QTableWidget>
#include <QTimer>

// ❌ 错误：漏了会编译失败
```

### 3. 每个 .cpp 函数必须在 .h 声明

```cpp
// mainwindow.h
private:
    void sendPowerParam(int addr, const QString &valueStr);  // 必须声明

// mainwindow.cpp
void MainWindow::sendPowerParam(int addr, const QString &valueStr)
{
    // 实现...
}
```

### 4. Qt 5/6 兼容

Qt 6 中 `QSerialPort::error` 改名为 `errorOccurred`：

```cpp
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &SerialPortManager::onError);
#else
    connect(m_serialPort, QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::error),
            this, &SerialPortManager::onError);
#endif
```

### 5. UI 文件标签闭合

每个 `<widget>` 必须有对应的 `</widget>` 闭合：

```xml
<!-- ✅ 正确 -->
<item>
  <widget class="QLineEdit" name="lineEdit_P1">
    <property name="text">
      <string></string>
    </property>
  </widget>
</item>

<!-- ❌ 错误：缺少 </widget> -->
<item>
  <widget class="QLineEdit" name="lineEdit_P1">
    <property name="text">
      <string></string>
    </property>
  <item>
```

### 6. 验证 UI 文件

生成或修改 UI 后，运行 uic 验证：

```bash
uic mainwindow.ui -o ui_mainwindow.h
```

无报错说明格式正确。

---

## 在 Qt Creator 中运行

1. 打开 `digitaluartcontrol.pro` 或 `*.pro`
2. 如有错误先安装 SerialPort
3. 构建 → 重新构建所有
4. 运行

---

## 技术栈

- **框架**: Qt 5.x / Qt 6.x
- **语言**: C++
- **串口**: QSerialPort
- **协议**: 自定义 UART 协议（帧头 + 长度 + 命令 + 地址 + 数据 + 校验）

## License

MIT License - 可以自由使用、修改和商用。

## 提示词

### 使用提示词

用 https://github.com/aninstone/uart-customized-protocol-skill 作为模板，
在 [D:\WorkPRJ\QT PRJ] 创建一个 UART 上位机工程。

### 操作步骤

1. **克隆仓库**
   ```
   git clone https://github.com/aninstone/uart-customized-protocol.git [目标路径]
   ```

2. **复制模板文件**
   ```
   cd [目标路径]
   mv assets/* ./
   rm -rf assets/
   ```

3. **重命名项目文件**
   ```
   # 删除 modbus 相关文件（如果存在）
   rm -f modbusrtumaster.h modbusrtumaster.cpp

   # 重命名 pro 文件
   mv digitaluartcontrol.pro [工程名].pro
   ```

4. **修改 .pro 文件**
   - 删除 modbusrtumaster 相关行
   - 添加 `TARGET = [工程名]`

5. **修改 mainwindow.ui 标题**
   ```
   <string>Digital UART Control</string>
   替换为
   <string>[工程名] 上位机</string>
   ```

6. **验证 UI 文件**
   ```
   uic mainwindow.ui -o ui_mainwindow.h
   ```

### 功能要求

1. 基于 digitaluartcontrol 项目结构（已包含在 assets 中）
2. 协议参数可在界面配置（帧头、设备地址、读/写/状态命令码、校验方式）
3. 深色主题 UI（#2b2b2b 背景，#00ff00 日志，#4a9eff 强调色）
4. 三个页面：手动控制、电源参数、运行监控
5. 修改 mainwindow.h/cpp/ui 三个文件
6. 确保所有函数在 .h 中声明
7. Qt 5/6 兼容
8. UI 标签正确闭合
9. 生成后运行 uic mainwindow.ui 验证

---

## 作者

GitHub: [@aninstone](https://github.com/aninstone)
