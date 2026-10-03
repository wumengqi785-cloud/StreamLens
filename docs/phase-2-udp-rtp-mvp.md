# 阶段二：UDP/RTP 视频流 MVP

## 阶段目标

在现有工程基础上，实现一个可以接收 UDP 数据、解析基础 RTP 信息、统计链路状态并原样转发数据的最小可用版本。

本阶段暂不实现完整 RTSP 客户端、视频解码、视频编码和硬件加速，重点是掌握 C++ 网络编程、线程、数据结构、资源管理和可测试的工程设计。

## 总体数据流

```text
UDP 输入端口
      ↓
接收器 UdpReceiver
      ↓
RTP 解析器 RtpParser
      ↓
统计模块 StreamStatistics
      ↓
UDP 输出端口
```

## 子目标一：命令行参数

### 目标

让程序能够通过命令行指定输入端口、输出地址和统计周期。

### 示例

```bash
streamlens --input-port 5000 --output-host 127.0.0.1 --output-port 9000 --stats-interval 1
```

### 验收标准

- 可以读取输入端口
- 可以读取输出主机和端口
- 参数缺失或非法时给出清晰错误
- `--help` 可以显示使用说明
- `--version` 可以显示版本号

## 子目标二：UDP 接收器

### 目标

实现一个负责 UDP Socket 创建、绑定和数据接收的模块。

### 建议接口

```cpp
class UdpReceiver {
public:
    bool open(uint16_t port);
    int receive(std::span<std::byte> buffer);
    void close();
};
```

如果当前编译环境不方便使用 `std::span`，第一版可以使用指针和长度参数。

### 验收标准

- 能够绑定指定 UDP 端口
- 能够接收数据报
- 能够处理 Socket 错误
- 能够通过停止信号安全退出
- 不发生明显的资源泄漏

## 子目标三：基础 RTP Header 解析

### 目标

解析 RTP 固定 Header 中最重要的字段，不处理完整的 RTP 扩展和复杂 Payload。

### 第一版字段

- Version
- Padding
- Extension
- CSRC Count
- Marker
- Payload Type
- Sequence Number
- Timestamp
- SSRC

### 建议结构

```cpp
struct RtpHeader {
    uint8_t version;
    bool padding;
    bool extension;
    bool marker;
    uint8_t payload_type;
    uint16_t sequence_number;
    uint32_t timestamp;
    uint32_t ssrc;
};
```

### 验收标准

- 能够拒绝长度不足的无效数据包
- 能够正确处理网络字节序
- 能够解析标准 RTP 固定 Header
- 有针对有效包和无效包的单元测试

## 子目标四：统计模块

### 目标

统计接收数据的基本链路指标。

### 第一版指标

- 接收数据包数量
- 接收字节数
- 估算码率
- RTP Sequence Number 连续性
- 丢包数量
- 丢包率
- 首包时间和运行时长
- Payload Type 分布

### 验收标准

- 统计模块不直接依赖 Socket
- 可以通过测试数据独立测试
- 周期性输出统计信息
- 没有数据时不会发生除零错误
- Sequence Number 回绕时不会误报大量丢包

## 子目标五：UDP 原样转发

### 目标

将接收到的 UDP 数据报原样发送到指定目标地址。

### 验收标准

- 输出端收到的数据与输入数据内容一致
- 转发失败时有错误日志
- 输入和输出地址可以独立配置
- 输出端关闭时程序可以安全退出

## 子目标六：流水线和线程模型

### 目标

将接收、处理、统计和转发组织成清晰的模块，避免所有逻辑堆积在 `main.cpp`。

### 推荐初始线程模型

```text
接收线程
   ↓
有界线程安全队列
   ↓
处理/统计线程
   ↓
转发线程
```

第一版也可以先采用单线程实现，确认功能后再拆分线程。不要为了使用多线程而过早增加复杂性。

### 验收标准

- 每个线程有明确职责
- 程序可以通过原子停止标志退出
- 队列有最大容量限制
- 停止时能够唤醒等待线程
- 线程能够正确回收

## 子目标七：日志和优雅退出

### 目标

建立基本运行日志，支持正常退出和错误诊断。

### 日志级别

- INFO：启动、连接、统计
- WARN：丢包、队列满、短暂发送失败
- ERROR：Socket 创建失败、绑定失败、不可恢复错误

### 验收标准

- Ctrl+C 可以触发安全退出
- Socket、线程和队列都能正确清理
- 错误日志包含足够的上下文
- 不使用不可控的强制终止方式

## 子目标八：测试工具和可重复验证

### 目标

建立不依赖真实摄像头的本地测试方式。

### 建议测试方式

使用 Python、FFmpeg 或简单 UDP 发送器生成测试数据：

```text
测试数据发送器 → StreamLens 输入端口 → StreamLens 输出端口 → 接收验证器
```

### 必须覆盖的场景

1. 正常连续发送
2. 空数据包或长度不足的数据包
3. Sequence Number 连续数据
4. Sequence Number 跳变
5. 大量数据快速发送
6. 输出端不可用
7. Ctrl+C 退出

## 推荐实现顺序

```text
命令行参数
   ↓
UDP 接收
   ↓
UDP 转发
   ↓
RTP Header 解析
   ↓
统计模块
   ↓
日志和优雅退出
   ↓
线程安全队列
   ↓
自动化测试
```

建议每完成一个子目标就提交一次 Git：

```text
feat: add command line argument parsing
feat: add UDP receiver
feat: add UDP packet relay
feat: add RTP header parser
feat: add stream statistics
test: add RTP parser tests
```

## 阶段二完成标准

阶段二完成时，StreamLens 应当能够：

- 从指定 UDP 端口接收数据
- 解析 RTP 基础 Header
- 统计数据包、字节数、码率和丢包率
- 将数据原样转发到指定目标
- 输出周期性运行状态
- 处理无效数据和 Socket 错误
- 通过 Ctrl+C 安全退出
- 通过自动化测试验证关键模块
- 在 WSL 中使用 CMake 完成构建和测试

## 暂不纳入阶段二的内容

- 完整 RTSP 客户端
- H.264/H.265 解码
- 视频编码
- WebRTC
- SRT
- 硬件编解码
- ARM 交叉编译
- Web 管理界面
- 复杂的插件系统

这些内容留到后续阶段，避免第一版范围失控。
