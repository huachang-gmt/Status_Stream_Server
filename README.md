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

---

# [2026-09-21] 更新 - 增加 封裝 Packet 

# Status Stream TCP Server

## 1. 目前開發目標

本功能是在 STM32H755 CM4 上建立一個非阻塞的 Status Stream TCP Server。

目前架構：

```text
STM32H755 CM4
    │
    │ Status Data
    ▼
68-byte Raw Status Data
    │
    ▼
Status Payload Builder
    │
    ▼
139-byte Routine Payload
    │
    ▼
Status Transport Packet Builder
    │
    ▼
151-byte Transport Packet
    │
    ▼
LwIP TCP Server
    │
    │ TCP Port 8888
    ▼
Windows TCP Client
```

STM32 TCP Server：

* IP：`192.168.137.10`
* TCP Port：`8888`
* LwIP：`2.2.1`
* Status Stream Server 執行於 STM32H755 CM4
* TCP Server 採非阻塞方式運作
* Status Packet 目前測試週期為 `1000 ms`
* 正式版本預計恢復為 `200 ms`

---

# 2. 前一階段已完成：139-byte Routine Payload

Status Stream 文件定義的 Status Data 為 68 bytes。

目前 STM32 已完成：

```text
68-byte Raw Status Data
        │
        ▼
StatusTCPServer_BuildPayload()
        │
        ▼
139-byte Routine Payload
```

139-byte Payload 格式：

```text
Byte 0
  │
  ├── '>'                       1 byte
  │
  ├── 136 ASCII HEX characters 136 bytes
  │
  └── "\r\n"                    2 bytes

Total = 139 bytes
```

因此：

```text
1 + 136 + 2 = 139 bytes
```

Raw Status Data 也可以由同一個 Payload Builder 轉換成 ASCII HEX。

目前支援：

```text
STATUS_DATA_RAW_68
STATUS_DATA_HEX_136
```

因此未來如果同事提供：

```text
68-byte Raw Data
```

可以直接轉換。

如果同事未來直接提供：

```text
136-byte ASCII HEX Data
```

也可以直接放入 139-byte Routine Payload。

---

# 3. 為什麼還需要 Transport Packet？

139-byte Routine Payload 本身可以正確表示 Status Data，但是 TCP 本身是 Byte Stream。

TCP 不提供：

```text
Packet Start
Packet End
Packet Length
Packet Sequence
Packet CRC
```

因此 Windows Client 不能假設：

```text
TCP Receive() == 一個完整的 139-byte Status
```

例如同一個 TCP Packet 可能被分成：

```text
50 bytes
+
89 bytes
```

也可能一次收到：

```text
139 + 139 + 139 bytes
```

因此在 139-byte Routine Payload 外面，再建立一層我們自己的 Transport Packet。

這一層不屬於原始 Status Stream 文件，而是本專案為了可靠處理 TCP Byte Stream 所定義的內部傳輸格式。

---

# 4. Status Transport Packet 架構

目前定義的 Transport Packet：

```text
┌───────────────┬───────────────────────────────┬───────────┐
│    Header     │           Payload             │   CRC16   │
│    10 bytes   │           139 bytes           │ 2 bytes   │
└───────────────┴───────────────────────────────┴───────────┘
│<----------------------- 151 bytes ----------------------->│
```

總長度：

```text
10 + 139 + 2 = 151 bytes
```

---

# 5. 151-byte Packet 詳細架構

```text
Byte 0
   │
   ▼
┌──────┬──────┬─────────┬─────────┬──────────────┬────────────┬───────────────────────────────┬───────────┐
│  47  │  53  │   01    │   01    │ Sequence     │   Length   │           Payload             │   CRC16   │
│      │      │         │         │   4 bytes    │  2 bytes   │          139 bytes            │  2 bytes  │
└──────┴──────┴─────────┴─────────┴──────────────┴────────────┴───────────────────────────────┴───────────┘
   0      1        2         3          4~7           8~9             10~148                    149~150
```

---

# 6. Header 欄位說明

## Byte 0 — Magic 0

```text
0x47
```

ASCII：

```text
'G'
```

用途：

讓 Client 可以尋找 Packet 起始位置。

---

## Byte 1 — Magic 1

```text
0x53
```

ASCII：

```text
'S'
```

因此 Packet 開頭固定：

```text
47 53
```

也就是：

```text
"GS"
```

Windows Client 如果在 TCP Stream 中搜尋 Packet，可以使用：

```text
47 53
```

作為 Frame Synchronization。

---

# 7. Byte 2 — Version

```text
0x01
```

目前：

```text
Version = 1
```

未來如果 Transport Packet 格式需要修改，可以透過 Version 區分不同版本。

---

# 8. Byte 3 — Type

目前：

```text
0x01
```

定義：

```text
0x01 = STATUS
```

因此目前 Header 前四 bytes 固定為：

```text
47 53 01 01
```

---

# 9. Byte 4~7 — Sequence

Sequence 使用：

```text
uint32_t
```

共 4 bytes。

目前使用 Little-Endian。

例如：

```text
Sequence = 0
```

封裝：

```text
00 00 00 00
```

Sequence = 1：

```text
01 00 00 00
```

Sequence = 10：

```text
0A 00 00 00
```

Sequence 每成功建立一個 Status Packet 後遞增。

例如：

```text
Sample 0 → Sequence 0
Sample 1 → Sequence 1
Sample 2 → Sequence 2
Sample 3 → Sequence 3
Sample 4 → Sequence 4
Sample 0 → Sequence 5
...
```

Sequence 可以讓 Windows Client 判斷封包順序。

---

# 10. Byte 8~9 — Payload Length

目前固定：

```text
139 bytes
```

Little-Endian：

```text
139 decimal = 0x008B

Packet:
8B 00
```

因此目前 Header 的前 10 bytes：

```text
47 53 01 01 XX XX XX XX 8B 00
```

---

# 11. Byte 10~148 — Payload

Payload 就是之前已經完成並驗證的：

```text
139-byte Routine Payload
```

內容：

```text
Byte 0:
'>'

Byte 1~136:
136 ASCII HEX characters

Byte 137:
'\r'

Byte 138:
'\n'
```

在 Transport Packet 中：

```text
Packet[10] ~ Packet[148]
```

就是完整的 139-byte Routine Payload。

Transport Layer 不重新解釋 Payload 內容。

也就是：

```text
Transport Packet
      │
      └── Payload
            │
            └── 原本已驗證的 139-byte Routine Packet
```

這樣可以讓 Transport Layer 與 Status Data Layer 分離。

---

# 12. Byte 149~150 — CRC16

目前使用本專案自行定義的 CRC16：

```text
Polynomial : 0xA001
Initial    : 0xFFFF
RefIn      : True
RefOut     : True
XorOut     : 0x0000
```

CRC 計算範圍：

```text
Packet[0] ~ Packet[148]
```

也就是：

```text
Header + Payload
```

不包含 CRC 本身。

CRC 佔最後 2 bytes：

```text
Packet[149]
Packet[150]
```

目前採 Little-Endian 儲存：

```text
Low Byte
High Byte
```

---

# 13. 完整 Packet 範例

例如 Sequence = 0：

```text
47 53 01 01 00 00 00 00 8B 00
```

代表：

```text
47 53
   Magic = GS

01
   Version = 1

01
   Type = STATUS

00 00 00 00
   Sequence = 0

8B 00
   Payload Length = 139
```

接下來：

```text
[139-byte Routine Payload]
```

最後：

```text
[2-byte CRC16]
```

因此：

```text
┌──────────────┬──────────────────────┬──────────┐
│ Header 10 B  │ Payload 139 B        │ CRC 2 B  │
└──────────────┴──────────────────────┴──────────┘
       10                139                2

                    = 151 bytes
```

---

# 14. STM32 Server 的封裝流程

目前 STM32 Server 的流程：

```text
status_raw_data_buffers[]
        │
        │ 68 bytes
        ▼
StatusTCPServer_BuildPayload()
        │
        │ 139 bytes
        ▼
status_payload_buffers[]
        │
        ▼
StatusTCPServer_BuildPacket()
        │
        │ 151 bytes
        ▼
status_packet_buffer[]
        │
        ▼
tcp_write()
        │
        │ 151 bytes
        ▼
TCP
```

---

# 15. 已完成的 STM32 Server 功能

目前以下功能已完成並實際測試：

### 15.1 Raw Data → Payload

```text
68 bytes → 139 bytes
```

PASS。

### 15.2 Payload → Transport Packet

```text
139 bytes → 151 bytes
```

PASS。

### 15.3 Header

已驗證：

```text
Magic   = 47 53
Version = 01
Type    = 01
Length  = 8B 00
```

PASS。

### 15.4 Sequence

已驗證：

```text
0 → 1 → 2 → 3 → 4 → 5 → ...
```

PASS。

### 15.5 CRC16

已完成 CRC16 計算並加入 Packet。

目前測試結果包含：

```text
Sequence 0  → CRC EE15
Sequence 1  → CRC 2E13
Sequence 2  → CRC 95E9
Sequence 3  → CRC FAD5
Sequence 4  → CRC ACF7
```

PASS。

### 15.6 Sample 循環

目前測試資料：

```text
Sample 0
Sample 1
Sample 2
Sample 3
Sample 4
Sample 0
...
```

PASS。

### 15.7 TCP 傳送

目前已由原本：

```text
139 bytes
```

改成：

```text
151 bytes
```

並實際由 STM32 TCP Server 傳送。

PASS。

---

# 16. 目前測試週期

目前：

```c
#define STATUS_TCP_PERIOD_MS 1000U
```

設定為 1000 ms 是為了開發與除錯時方便觀察。

正式版本預計恢復：

```c
#define STATUS_TCP_PERIOD_MS 200U
```

目前不在這個 checkpoint 修改。

---

# 17. TCP Server 目前狀態

目前 STM32 Server 已經完成：

```text
Status Data
    ↓
139-byte Payload
    ↓
151-byte Transport Packet
    ↓
TCP Server
```

因此 STM32 Server 端的 Transport Layer 已完成第一階段驗證。

---

# 18. 尚未完成的部分

整個 Status Stream 系統目前尚未全部完成。

下一階段為 Windows TCP Client：

```text
TCP
 ↓
Receive Byte Stream
 ↓
Stream Buffer
 ↓
搜尋 47 53
 ↓
讀取 Header
 ↓
取得 Payload Length
 ↓
等待完整 Packet
 ↓
CRC16 驗證
 ↓
Sequence 驗證
 ↓
取出 139-byte Payload
 ↓
Status Parser
 ↓
Dashboard
```

Windows Client 必須特別處理 TCP Byte Stream。

不能假設：

```text
TCP Read = 151 bytes
```

---

# 19. Windows Client 必須測試的 TCP 情況

## Case 1 — 完整 Packet

```text
151 bytes
```

Client 應直接解析一個 Packet。

---

## Case 2 — Packet Fragmentation

例如：

```text
50 bytes
+
101 bytes
```

Client 必須累積：

```text
50 + 101 = 151
```

之後再解析 Packet。

---

## Case 3 — Multiple Packets

例如：

```text
151 + 151 + 151
```

一次收到：

```text
453 bytes
```

Client 必須解析：

```text
Packet 1
Packet 2
Packet 3
```

---

## Case 4 — Incomplete Packet Timeout

例如只收到：

```text
100 bytes
```

之後 TCP 接收 Timeout。

Client 應：

```text
丟棄 incomplete packet
等待下一個 Status Packet
```

本 Status Stream 不設計 retransmission。

---

# 20. 目前 GitHub Checkpoint

本階段建議建立 GitHub checkpoint：

```text
Status Stream Transport Packet Validation Passed
```

Checkpoint 包含：

```text
[PASS] 68-byte Raw Status Data
[PASS] 139-byte Routine Payload
[PASS] 151-byte Transport Packet
[PASS] Header
[PASS] Sequence
[PASS] CRC16
[PASS] Sample 0~4 cycling
[PASS] TCP Server sends 151-byte packet
```

注意：

這個 checkpoint 代表的是：

> **STM32H755 CM4 Status Stream TCP Server + Transport Packet 已完成並驗證。**

並不代表整個 Status Stream 專案全部完成。

下一個開發階段是：

```text
Windows TCP Client
```

負責接收並解析上述 151-byte Transport Packet。

### GitHub 建議

現在就建立 checkpoint，**不要等 Windows Client 做完才 commit**。

這個 checkpoint 很有價值，因為我們現在已經有一個很清楚的基準：

```text
STM32 Server
    ↓
151-byte Packet
    ↓
TCP
```

之後 Windows Client 不論怎麼修改，都可以拿這個版本當作「Server 已驗證」的基準。

另外，目前 `STATUS_TCP_PERIOD_MS = 1000U` **先維持不要改**；等整個 Status Stream Server + Windows Client + Dashboard 都驗證完成，再做最後的正式化，包括恢復 `200U`、清理測試 sample、確認正式介面。

這一版本還沒有建立完整的 TCP Client ， 使用 Powershell 執行以下 script 來啟動 TCP Server

```bash
$client = New-Object System.Net.Sockets.TcpClient
$client.Connect("192.168.137.10", 8888)
```
STM32H755 TCP Server 端的輸出：

```text
===== CM4 TCP Status Stream Server =====
[STATUS] Sample 0 BuildPayload OK
[STATUS] Payload[0..20]: >0000000340000000199A
[STATUS] Packet Build OK
[STATUS] Packet Length = 151
[STATUS] Sequence = 0
[STATUS] Payload Length = 139
[STATUS] CRC = EE15
[STATUS] Header: 47 53 01 01 00 00 00 00 8B 00
[STATUS] Sample 0 TCP send OK, 151 bytes

[STATUS] Sample 1 BuildPayload OK
[STATUS] Payload[0..20]: >0000000B600000003333
[STATUS] Packet Build OK
[STATUS] Packet Length = 151
[STATUS] Sequence = 1
[STATUS] Payload Length = 139
[STATUS] CRC = 2E13
[STATUS] Header: 47 53 01 01 01 00 00 00 8B 00
[STATUS] Sample 1 TCP send OK, 151 bytes

[STATUS] Sample 2 BuildPayload OK
[STATUS] Payload[0..20]: >00000013800000004CCC
[STATUS] Packet Build OK
[STATUS] Packet Length = 151
[STATUS] Sequence = 2
[STATUS] Payload Length = 139
[STATUS] CRC = 95E9
[STATUS] Header: 47 53 01 01 02 00 00 00 8B 00
[STATUS] Sample 2 TCP send OK, 151 bytes

[STATUS] Sample 3 BuildPayload OK
[STATUS] Payload[0..20]: >0000002BBFFF00006666
[STATUS] Packet Build OK
[STATUS] Packet Length = 151
[STATUS] Sequence = 3
[STATUS] Payload Length = 139
[STATUS] CRC = FAD5
[STATUS] Header: 47 53 01 01 03 00 00 00 8B 00
[STATUS] Sample 3 TCP send OK, 151 bytes

[STATUS] Sample 4 BuildPayload OK
[STATUS] Payload[0..20]: >00000023E66600008000
[STATUS] Packet Build OK
[STATUS] Packet Length = 151
[STATUS] Sequence = 4
[STATUS] Payload Length = 139
[STATUS] CRC = ACF7
[STATUS] Header: 47 53 01 01 04 00 00 00 8B 00
[STATUS] Sample 4 TCP send OK, 151 bytes

```
---
