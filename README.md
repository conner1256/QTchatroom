实验日志
日期: 2025-12-27
项目名称: Qt 网络聊天室
环境: Qt 5.15+, C++11

1. 项目搭建与界面设计

创建了 ChatRoom subdirs 项目，包含 Server 和 Client 两个子项目。
Server: 设计了包含日志显示区、端口设置和启动按钮的界面。
Client: 设计了包含聊天记录显示区、消息输入框、发送按钮、连接设置区（IP、端口、昵称）以及在线用户列表的界面。
遇到问题：布局管理器初期设置比例不当，导致窗口拉伸时界面变形。解决：使用 setStretchFactor 调整 Layout 比例。
2. 基本槽函数与调试

实现了 Server 的启动、新连接、断开连接、读取数据的槽函数。
实现了 Client 的连接、发送、接收消息的槽函数。
使用 qDebug() 在控制台输出原始发送和接收的数据，验证 TCP 连接通畅。
3. 通讯实现

测试了 Server 与 Client 的点对点连接。
测试了 Client 发送字符串，Server 能够在 Log 窗口回显。
遇到问题：中文消息在传输过程中可能出现乱码。解决：统一使用 toUtf8() 和 fromUtf8() 进行编码转换。
4. JSON 数据协议设计

定义了 login, msg, system, userlist 四种 JSON 消息类型。
引入了 QJsonDocument 和 QJsonObject 进行数据的封装和解析。
Server 端成功解析了 Client 发送的登录报文，并能提取 name 字段。
5. 功能完善

登录逻辑: Client 连接成功后自动发送登录包，Server 接收后更新用户属性。
消息转发: Server 接收到 msg 类型报文后，将其转发给所有连接的 Client。
用户列表: Server 维护 clientList，当有用户登录或退出时，重新生成用户列表 JSON 并广播给所有客户端。Client 解析 userlist 报文并刷新右侧 QListWidget。
异常处理: 处理了客户端强制关闭的情况，Server 能够触发 disconnected 信号，从列表中移除该 Socket 并通知其他用户。
6. 多客户端测试

启动一个 Server，同时启动了三个 Client 实例（User1, User2, User3）。
验证了 User1 发送消息，User2 和 User3 均能收到。
验证了 User3 退出，User1 和 User2 的用户列表中自动移除 User3，并显示系统提示“User3 离开了聊天室”。
总结:
本次实验成功实现了一个基于 Qt TCP Socket 和 JSON 协议的多人聊天室。掌握了 Qt 的网络编程接口、信号槽机制、JSON 数据处理以及多客户端并发连接的管理方法。代码结构清晰，界面交互友好，基本功能运行稳定。
