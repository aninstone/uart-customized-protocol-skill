用 https://github.com/aninstone/digitaluartcontrol 作为模板，
在 [目标路径] 创建一个 UART 上位机工程。

要求：
1. 基于 digitaluartcontrol 项目结构
2. 协议参数可在界面配置（帧头、设备地址、读/写/状态命令码、校验方式）
3. 深色主题 UI（#2b2b2b 背景，#00ff00 日志，#4a9eff 强调色）
4. 三个页面：手动控制、电源参数、运行监控
5. 修改 mainwindow.h/cpp/ui 三个文件
6. 确保所有函数在 .h 中声明
7. Qt 5/6 兼容
8. UI 标签正确闭合
9. 生成后运行 uic mainwindow.ui 验证
