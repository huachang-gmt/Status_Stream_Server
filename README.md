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
# [2026-09-18] 更新版本

# STM32H755 Status Stream TCP Server — Validation Version

## 1. 本階段目的

本階段的目的，是先在 STM32H755 端完成並驗證 Status Stream 的基本資料流程：

```text
68-byte Raw Status Data
        ↓
StatusTCPServer_BuildPayload()
        ↓
139-byte Status Payload
        ↓
TCP Server
        ↓
TCP Client
```

本階段使用五組固定的 Sample Data 進行 Validation。

目的不是完成最終正式版本，而是先確認：

* 68-byte Raw Data 可以正確轉換成 139-byte Payload
* Payload 格式正確
* 五組 Sample 可以依序循環
* STM32 TCP Server 可以正常傳送 Payload
* PC TCP Client 可以正確收到完整的 139-byte Payload
* STM32 端 `printf` 與 PC 端接收資料可以互相對應

本階段完成並確認無誤後，才進入下一階段的 151-byte Status Transport Packet 開發。

---

# 2. Validation Version 的資料格式

## 2.1 Raw Data

每組 Sample：

```text
68 bytes
```

目前共有：

```text
Sample 0
Sample 1
Sample 2
Sample 3
Sample 4
```

---

## 2.2 Status Payload

Raw Data 會經過：

```text
68-byte Raw Data
        ↓
136-byte ASCII HEX
        ↓
'>' + 136-byte HEX + "\r\n"
        ↓
139-byte Payload
```

因此 Payload 固定為：

```text
139 bytes
```

格式：

```text
> + 136 ASCII HEX characters + CRLF
```

也就是：

```text
Byte 0       : '>'
Byte 1~136   : ASCII HEX
Byte 137     : '\r'
Byte 138     : '\n'
```

---

# 3. Validation Version 的 Sample

目前使用五組固定 Sample Data：

| Sample | Controller Status |  X |  Y |  Z | RX | RY | RZ |
| ------ | ----------------: | -: | -: | -: | -: | -: | -: |
| 0      |        0x00000003 |  0 |  0 |  0 |  0 |  0 |  0 |
| 1      |        0x0000000B | 10 | 20 | 30 |  1 |  2 |  3 |
| 2      |        0x00000013 | 20 | 10 |  5 |  2 |  4 |  6 |
| 3      |        0x0000002B | 30 | 15 | 10 |  3 |  6 |  9 |
| 4      |        0x00000023 | 40 | 20 | 15 |  4 |  8 | 12 |

這五組資料刻意使用不同的 Controller Status、Analog Input 與 Position 數值，方便在 PC Client 上確認資料確實有變化。

---

# 4. Sample 循環條件

Validation Version 的 Sample 順序固定為：

```text
Sample 0
   ↓
Sample 1
   ↓
Sample 2
   ↓
Sample 3
   ↓
Sample 4
   ↓
Sample 0
   ↓
...
```

目前 Validation Version 的傳送週期設定為：

```c
#define STATUS_TCP_PERIOD_MS 1000U
```

也就是每：

```text
1000 ms = 1 second
```

切換一次 Sample。

### 為什麼不是正式的 200ms？

這是刻意的 Validation 設定。

1000ms 可以讓工程師在 STM32 `printf` Console 與 PC Client 上清楚看到：

```text
0 → 1 → 2 → 3 → 4 → 0
```

正式版本完成後會恢復：

```c
#define STATUS_TCP_PERIOD_MS 200U
```

---

# 5. 五組測試條件

本階段使用以下五組 Sample 作為固定 Regression Test。

## Test 0 — Sample 0

Expected Payload 開頭：

```text
>0000000340000000199A
```

確認：

* Sample 0 可以正常建立
* Controller Status 正確
* Analog Input 資料正確
* Position 全部為 0
* Payload 開頭為 `>`
* Payload 長度為 139 bytes

---

## Test 1 — Sample 1

Expected Payload 開頭：

```text
>0000000B600000003333
```

確認：

* Sample 1 可以正常建立
* Controller Status 發生變化
* Analog Input 資料正確
* Position 數值開始變化
* Payload 與 Sample 0 不相同

---

## Test 2 — Sample 2

Expected Payload 開頭：

```text
>00000013800000004CCC
```

確認：

* Sample 2 可以正常建立
* Controller Status 正確
* Analog Input 資料正確
* X/Y/Z 與 Rotation 數值持續變化

---

## Test 3 — Sample 3

Expected Payload 開頭：

```text
>0000002BBFFF00006666
```

確認：

* Sample 3 可以正常建立
* Controller Status 正確
* Analog Input 資料正確
* Position 資料正確

---

## Test 4 — Sample 4

Expected Payload 開頭：

```text
>00000023E66600008000
```

確認：

* Sample 4 可以正常建立
* Controller Status 正確
* Analog Input 資料正確
* Position 資料正確
* 五組 Sample 的最後一組可以正常傳送

完成 Test 4 後，下一次必須重新回到：

```text
Sample 0
```

以確認循環功能正常。

---

# 6. STM32 端 printf Validation

STM32 端會輸出以下資訊：

```text
[STATUS] Sample 0 BuildPayload OK
[STATUS] Payload[0..20]: >0000000340000000199A
[STATUS] Sample 0 TCP send OK, 139 bytes
```

這三行代表：

### BuildPayload OK

```text
[STATUS] Sample 0 BuildPayload OK
```

表示：

```text
status_raw_data_buffers[0]
        ↓
StatusTCPServer_BuildPayload()
        ↓
status_payload_buffers[0]
```

建立成功。

---

### Payload[0..20]

例如：

```text
[STATUS] Payload[0..20]: >0000000340000000199A
```

表示 STM32 目前建立出的 Payload 前 21 bytes。

主要用來快速確認：

* `>` 是否存在
* HEX Conversion 是否正確
* Sample 是否正確

---

### TCP send OK

例如：

```text
[STATUS] Sample 0 TCP send OK, 139 bytes
```

表示：

```text
tcp_write()
```

成功接受這一筆 139-byte Payload。

---

# 7. TCP write FAILED 的意義

如果看到：

```text
[STATUS] Sample 1 TCP write FAILED
```

這不代表 `BuildPayload()` 失敗。

它代表 TCP Client 連線已經中斷或 TCP 傳送發生錯誤。

例如使用：

```powershell
Test-NetConnection 192.168.137.10 -Port 8888
```

只能測試 TCP Port 是否可以連線。

它不是持續接收 Status Stream 的 Client。

因此可能看到：

```text
[STATUS] Sample 0 BuildPayload OK
[STATUS] Sample 1 TCP write FAILED
```

這種情況屬於測試工具連線結束所造成的正常現象。

---

# 8. 最終 Validation 測試方法

本階段最終測試使用 Windows PowerShell 建立一個最小 TCP Client。

STM32 IP：

```text
192.168.137.10
```

Status TCP Server：

```text
Port 8888
```

在 Windows PowerShell 執行：

```powershell
$client = New-Object System.Net.Sockets.TcpClient
$client.Connect("192.168.137.10", 8888)

$stream = $client.GetStream()
$buffer = New-Object byte[] 1024

while ($true) {
    $count = $stream.Read($buffer, 0, $buffer.Length)

    if ($count -le 0) {
        break
    }

    $text = [System.Text.Encoding]::ASCII.GetString($buffer, 0, $count)

    Write-Host "[RX] $count bytes"
    Write-Host $text
}
```

此測試 Client 的目的非常單純：

```text
Connect
   ↓
保持 TCP Connection
   ↓
Receive
   ↓
顯示收到的資料
```

它不是正式的 Status Client。

---

# 9. Windows TCP Client 預期輸出

正常情況下應該看到：

```text
[RX] 139 bytes
>0000000340000000199A33334CCC666680009999000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
```

接著：

```text
[RX] 139 bytes
>0000000B6000000033334CCC666680009999B33240240000000000004034000000000000403E0000000000003FF000000000000040000000000000004008000000000000
```

然後：

```text
[RX] 139 bytes
>00000013800000004CCC666680009999B332CCCC403400000000000040240000000000004014000000000000400000000000000040100000000000004018000000000000
```

接著 Sample 3、Sample 4，最後回到 Sample 0。

---

# 10. 最終測試通過條件

本階段判定 PASS 必須同時符合以下條件。

### STM32 端

必須可以持續看到：

```text
Sample 0
Sample 1
Sample 2
Sample 3
Sample 4
Sample 0
...
```

而且每一組都出現：

```text
BuildPayload OK
```

以及：

```text
TCP send OK, 139 bytes
```

---

### Windows 端

必須持續收到：

```text
[RX] 139 bytes
```

並且 Payload 依序呈現：

```text
Sample 0
    ↓
Sample 1
    ↓
Sample 2
    ↓
Sample 3
    ↓
Sample 4
    ↓
Sample 0
```

---

# 11. 本階段已驗證的資料流程

本階段完成後，以下流程已經實際驗證：

```text
status_raw_data_buffers[]
        │
        │ 68-byte Raw Data
        ▼
StatusTCPServer_BuildPayload()
        │
        │ 136-byte ASCII HEX
        ▼
status_payload_buffers[]
        │
        │
        │ '>' + 136 HEX + CRLF
        ▼
139-byte Status Payload
        │
        ▼
tcp_write()
        │
        ▼
Ethernet TCP
        │
        ▼
Windows TCP Client
        │
        ▼
139-byte Payload
```

---

# 12. 本階段尚未完成的功能

以下功能**刻意留到下一階段**：

* 10-byte Status Transport Header
* Magic `0x47 0x53`
* Version
* Packet Type
* Sequence Number
* Payload Length
* CRC
* 151-byte完整 Transport Packet
* TCP Client 對 151-byte Packet 的解析
* 正式 200ms Status Stream
* 正式單一 Raw Buffer / Payload Buffer 架構

因此，本 README 對應的是：

```text
Validation Version
```

不是最終 Production Version。

---

# 13. Validation Checkpoint

本階段完成的核心驗證：

```text
[PASS] 68-byte Raw Data
[PASS] Raw → ASCII HEX conversion
[PASS] 139-byte Payload generation
[PASS] Five Sample validation
[PASS] Sample 0 → 1 → 2 → 3 → 4 → 0 loop
[PASS] STM32 TCP Server
[PASS] TCP transmission
[PASS] Windows TCP reception
[PASS] 139-byte Payload reception
```

本 Checkpoint 可作為後續正式 Status Stream 開發的基準版本。

如果後續 Transport Packet 開發發生問題，可以回到本 Checkpoint，重新確認：

```text
68-byte Raw Data
        ↓
139-byte Payload
        ↓
TCP
        ↓
PC
```

是否仍然正常。

這份 README 可以直接放進目前 GitHub checkpoint。

我建議這次 Git commit / checkpoint 的名稱就用：

```text
Status Stream Validation - 139-byte Payload TCP Test Passed
```


