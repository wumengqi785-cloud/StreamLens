# StreamLens 阶段性目标与验证方案

## 总体原则

每个阶段都必须同时具备：

```text
代码产出
   +
可运行程序
   +
自动化测试
   +
手工验证方法
   +
文档记录
```

每完成一个阶段，都应保留一个可以编译、运行和验证的版本，避免只完成代码而无法证明功能正确。

## 当前完成情况

```text
阶段一：工程基础       已完成
阶段二-1：命令行参数   已完成
阶段二-2：UDP 接收     已完成
阶段二-3：UDP 原样转发 已完成
```

当前数据链路：

```text
UDP 输入
   ↓
UdpReceiver
   ↓
包数和字节数统计
   ↓
UdpSender
   ↓
UDP 输出
```

## 阶段二-4：RTP Header 解析

### 目标

从 UDP 数据中解析 RTP 基础 Header。

### 实现内容

- RTP Version
- Padding
- Extension
- CSRC Count
- Marker
- Payload Type
- Sequence Number
- Timestamp
- SSRC
- Payload 起始位置

### 代码产出

```text
include/streamlens/rtp/rtp_header.h
include/streamlens/rtp/rtp_parser.h
src/rtp/rtp_parser.cpp
tests/rtp_parser_test.cpp
```

### 验证内容

使用 CTest 验证：

```bash
ctest --test-dir build --output-on-failure
```

测试必须覆盖：

- 合法 RTP 包
- 长度不足
- 错误 Version
- 网络字节序
- CSRC
- Header Extension
- Padding

### 完成标准

```text
输入一段 RTP 二进制数据
    ↓
正确解析 Header 字段
    ↓
正确定位 Payload
    ↓
无效数据返回清晰错误
```

## 阶段二-5：RTP 统计

### 目标

利用 RTP Header 统计视频链路基础指标。

### 实现内容

- RTP 包数量
- 接收字节数
- Payload Type 分布
- SSRC 记录
- Sequence Number 连续性
- 丢包数量
- 丢包率
- Sequence Number 回绕处理

### 代码产出

```text
include/streamlens/statistics/stream_statistics.h
src/statistics/stream_statistics.cpp
tests/stream_statistics_test.cpp
```

### 验证内容

使用测试序列：

```text
100, 101, 102, 105
```

预期：

```text
接收包数：4
检测丢包数：2
```

还必须测试正常回绕：

```text
65534, 65535, 0, 1
```

该序列不能被误报为大量丢包。

### 完成标准

```text
输入 RTP Header 序列
    ↓
输出正确的包数、丢包数和丢包率
```

## 阶段二-6：RTP 调试输出

### 目标

让程序实时显示 RTP 流状态。

### 实现内容

周期性输出：

```text
[STAT]
packets=1000
bytes=1280000
payload_type=96
ssrc=123456
loss=0.20%
bitrate=2.5Mbps
```

### 验证内容

使用 RTP 测试数据发送器向 StreamLens 发送数据，观察统计信息是否持续更新。

同时确认：

- 数据包数量持续增加
- 字节数持续增加
- Sequence Number 状态正确
- 模拟丢包时丢包率符合预期
- 转发数据仍然保持一致

## 阶段三：本地真实视频 RTP 流

### 目标

让 StreamLens 接收实际视频产生的 RTP 数据，而不是手工构造的数据包。

### 实现内容

- 使用 FFmpeg 生成视频 RTP 流
- 使用 MediaMTX 或其他本地服务
- 识别视频 Payload Type
- 统计视频帧率和码率
- 保持视频流原样转发

### 验证链路

```text
demo.mp4
   ↓ FFmpeg
RTSP/RTP 服务
   ↓
StreamLens
   ↓
FFplay/VLC
```

### 完成标准

- FFmpeg 能够产生输入视频流
- StreamLens 能够接收并解析 RTP
- 转发后的输出能够正常播放
- StreamLens 统计数据持续变化
- 输入输出数据包数量基本一致
- 转发没有破坏视频码流

## 阶段四：RTP 视频 Payload 分析

### 目标

开始处理和视频编码相关的 RTP Payload。

第一版只支持 H.264，不进行视频解码。

### 实现内容

- H.264 Single NAL Unit
- FU-A 分片
- NALU 类型
- SPS/PPS 识别
- IDR 关键帧识别
- 帧边界统计

### 代码产出

```text
include/streamlens/video/h264_payload_parser.h
src/video/h264_payload_parser.cpp
tests/h264_payload_parser_test.cpp
```

### 验证内容

输出示例：

```text
codec=H264
nalu_type=5
keyframe=true
fragmented=false
```

### 完成标准

- 能识别 SPS
- 能识别 PPS
- 能识别 IDR
- 能识别 FU-A
- 不修改原始转发数据
- 无效 Payload 能安全拒绝

## 阶段五：异常检测和自动重连

### 目标

模拟真实设备端视频链路中的异常情况。

### 实现内容

- 接收超时
- 断流检测
- RTP 序列异常
- SSRC 变化检测
- 长时间无数据检测
- 自动重连
- 异常日志

### 验证步骤

1. 启动输入视频流。
2. 启动 StreamLens。
3. 停止输入流。
4. 等待超时。
5. 查看断流日志。
6. 恢复输入流。
7. 查看自动恢复结果。

预期日志：

```text
[WARN] RTP stream timeout
[WARN] source disconnected
[INFO] reconnecting
[INFO] stream recovered
```

### 完成标准

- 停流能够被检测
- 恢复后能够重新工作
- 程序不崩溃
- 资源能够释放
- 重连次数有统计

## 阶段六：多路视频流

### 目标

支持同时处理多路视频流。

### 实现内容

- 多路输入配置
- 每路独立统计
- 每路独立输出
- 每路独立 SSRC 状态
- 资源限制
- 流之间互不影响

### 验证链路

```text
输入流 A → StreamLens → 输出流 A
输入流 B → StreamLens → 输出流 B
```

### 完成标准

- 两路画面都能播放
- 统计信息相互独立
- 停止一路不影响另一路
- CPU 和内存没有异常增长

## 阶段七：工程质量和开源发布

### 目标

把项目整理成可长期维护的开源项目。

### 实现内容

- 完善 README
- 添加架构图
- 添加使用示例
- 完善单元测试
- 增加集成测试
- 配置 GitHub Actions
- 添加 clang-format
- 添加静态分析
- 添加 AddressSanitizer
- 添加开源许可证
- 发布 Release 版本

### 完成标准

新用户只需执行：

```bash
git clone <repository-url>
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

就可以完成构建和测试。

## 推荐推进顺序

```text
RTP Header 解析
    ↓
RTP 丢包统计
    ↓
RTP 调试输出
    ↓
真实视频 RTP 流
    ↓
H.264 Payload 分析
    ↓
断流与自动重连
    ↓
多路视频流
    ↓
工程质量和开源发布
```

## 下一步

下一步实现一个独立、可单元测试的 RTP Header 解析器。第一版只处理 RTP 层，不同时加入 H.264 解码、WebRTC、SRT 或硬件编解码，以保证每个阶段范围清晰且结果可验证。
