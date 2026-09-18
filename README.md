# GMT Status Stream

STM32H755 Status Stream TCP Server 開發專案。

本專案用於建立獨立的 Status Stream 通訊功能，與既有的 GMT Command Server / USB Command Transport 分開開發。

目前第一階段只驗證 STM32H755 CM4 Ethernet TCP Server 的基本功能，尚未加入正式 Status Stream 資料格式。

---

## 目前開發階段

### 第一階段：TCP Server 基礎功能

目前已完成：

* STM32H755 CM4 建立 TCP Server
* 使用 LwIP Raw TCP API
* TCP Server Port：`8888`
* STM32 IP：`192.168.137.10`
* PC IP：`192.168.137.1`
* PC 可以成功建立 TCP Connection
* TCP Server 可以接受 PC Client
* TCP Server 每約 200ms 傳送一次測試資料
* 測試資料：

```text
STATUS_TEST\r\n
```

* 已確認 TCP Server 可以持續傳送資料
* Server 使用 Non-Blocking 設計
* 尚未加入正式 Status Stream 資料格式

---

## 系統架構

目前第一階段架構：

```text
┌──────────────────────┐
│      PC / Client     │
│                      │
│  TCP Client          │
│  192.168.137.1       │
└──────────┬───────────┘
           │
           │ TCP : 8888
           │
┌──────────▼───────────┐
│    STM32H755 CM4     │
│                      │
│   LwIP TCP Server    │
│   192.168.137.10     │
│                      │
│ StatusTCPServer      │
└──────────────────────┘
```

---

## STM32 TCP Server

目前使用 LwIP Raw TCP API：

```text
tcp_new()
tcp_bind()
tcp_listen()
tcp_accept()
tcp_write()
tcp_output()
```

專案使用：

```text
NO_SYS = 1
LWIP_NETCONN = 0
LWIP_SOCKET = 0
```

因此沒有使用 BSD Socket 或 Netconn API。

---

## Non-Blocking 設計

Status TCP Server 不使用：

* HAL_Delay()
* Blocking Socket
* recv() 等待
* RTOS Thread
* TCP 傳送等待迴圈

主迴圈維持：

```c
while (1)
{
    MX_LWIP_Process();
    StatusTCPServer_Process();
}
```

`StatusTCPServer_Process()` 只在需要傳送資料時執行 TCP 傳送。

如果 TCP Send Buffer 空間不足，直接返回，不等待。

因此目前 Status TCP Server 採用 Non-Blocking 架構。

---

## 目前測試方式

### 1. 測試 TCP Port

Windows PowerShell：

```powershell
Test-NetConnection 192.168.137.10 -Port 8888
```

預期：

```text
TcpTestSucceeded : True
```

---

### 2. 測試 TCP Data Stream

Windows PowerShell：

```powershell
$client = New-Object System.Net.Sockets.TcpClient("192.168.137.10",8888)
$stream = $client.GetStream()
$buffer = New-Object byte[] 1024

while ($true) {
    $count = $stream.Read($buffer, 0, $buffer.Length)
    if ($count -le 0) { break }
    [System.Text.Encoding]::ASCII.GetString($buffer, 0, $count)
}
```

預期可以持續看到：

```text
STATUS_TEST
STATUS_TEST
STATUS_TEST
STATUS_TEST
...
```

---

## 專案檔案

目前 Status TCP Server 主要程式：

```text
CM4/
└── Core/
    ├── Inc/
    │   └── status_tcp_server.h
    │
    └── Src/
        ├── main.c
        └── status_tcp_server.c
```

### `status_tcp_server.h`

提供：

```c
void StatusTCPServer_Init(void);
void StatusTCPServer_Process(void);
```

### `status_tcp_server.c`

目前負責：

* TCP Server 初始化
* Port 8888 Listen
* TCP Client Accept
* Client 連線管理
* 每 200ms 傳送測試資料
* TCP Send Buffer 檢查
* Non-Blocking TCP 傳送

---

## 下一階段

下一階段開始導入正式的 **Status Stream Specification**。

預計流程：

```text
Status Stream 文件
        │
        ▼
確認資料格式與欄位定義
        │
        ▼
STM32H755 Status Data
        │
        ▼
Status Stream TCP Server
        │
        │ TCP : 8888
        ▼
PC Status Stream Client
        │
        ▼
解析 Status Stream
```

正式 Status Stream 的資料格式、欄位、長度、封包結構與資料來源，將以正式文件為準。

在正式文件確認之前，不預先定義 Status Stream 的資料內容。

---

## 開發原則

本專案與既有 GMT Command Server 分開開發。

目前：

```text
Command Server
    Port 9999
    ↓
Command Parser
    ↓
USB CDC
    ↓
STM32
```

與：

```text
Status Stream Server
    Port 8888
    ↓
Status Stream
    ↓
PC Status Client
```

為兩個不同功能。

Status Stream 專案將維持獨立，避免影響已完成的 Command / USB 通訊功能。

---

## 開發狀態

| 階段   | 功能                      | 狀態    |
| ---- | ----------------------- | ----- |
| 第一階段 | STM32 TCP Server        | ✅ 完成  |
| 第一階段 | Port 8888               | ✅ 完成  |
| 第一階段 | PC TCP Connection       | ✅ 完成  |
| 第一階段 | 200ms 測試資料傳送            | ✅ 完成  |
| 第一階段 | Non-Blocking 基礎架構       | ✅ 完成  |
| 第二階段 | Status Stream 正式格式      | ⏳ 待進行 |
| 第二階段 | PC Status Client        | ⏳ 待進行 |
| 第三階段 | EtherCAT Status Data 整合 | ⏳ 待進行 |

## 測試手法：
1. 開啟 Powershell
輸入 命令： 
```bash
Test-NetConnection 192.168.137.10 -Port 8888
```

2. 開啟 powershell 輸入以下命令：
```bash
$client = New-Object System.Net.Sockets.TcpClient("192.168.137.10",8888)
$stream = $client.GetStream()
$buffer = New-Object byte[] 1024

while ($true) {
    $count = $stream.Read($buffer, 0, $buffer.Length)
    if ($count -le 0) { break }
    [System.Text.Encoding]::ASCII.GetString($buffer, 0, $count)
}
```
輸出會得到：
```
STATUS_TEST
STATUS_TEST
STATUS_TEST
STATUS_TEST
.....一直持續下去
...
```
---
