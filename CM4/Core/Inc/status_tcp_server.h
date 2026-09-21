#ifndef STATUS_TCP_SERVER_H
#define STATUS_TCP_SERVER_H

#include <stdint.h>
#include <stdbool.h>

/*
 * ============================================================
 * Status TCP Server
 * ============================================================
 */

/* TCP server port */
#define STATUS_TCP_SERVER_PORT           8888U

/* Status stream transmission interval */
#define STATUS_TCP_PERIOD_MS             1000U   //200U


/*
 * ============================================================
 * Routine Status Packet
 * ============================================================
 *
 * Raw data layout:
 *
 *   Bytes 00 ~ 03 : Controller Status    UINT32   4 bytes
 *   Bytes 04 ~ 19 : Analog Input         UINT16  16 bytes
 *   Bytes 20 ~ 67 : Main Stages Position DOUBLE  48 bytes
 *
 *   Total raw data = 68 bytes
 *
 * Raw data is converted to HEX:
 *
 *   68 raw bytes -> 136 ASCII HEX characters
 *
 * Complete Routine Packet:
 *
 *   '>' + 136 HEX characters + "\r\n"
 *
 *   Total = 1 + 136 + 2 = 139 bytes
 */

/* Raw binary status data size */
#define STATUS_ROUTINE_RAW_SIZE          68U

/* HEX representation size */
#define STATUS_ROUTINE_HEX_SIZE          136U

/* Complete Routine Packet size */
#define STATUS_ROUTINE_PACKET_SIZE       139U

/*
 * ============================================================
 * Routine Status Data Type
 * ============================================================
 */

typedef enum
{
    STATUS_DATA_RAW_68 = 0,
    STATUS_DATA_HEX_136
} StatusDataType;


/*
 * ============================================================
 * Routine Status Payload Buffer
 * ============================================================
 */

#define STATUS_PAYLOAD_BUFFER_COUNT       5U

extern uint8_t status_payload_buffers[STATUS_PAYLOAD_BUFFER_COUNT]
                                      [STATUS_ROUTINE_PACKET_SIZE];
/*
 * ============================================================
 * Routine Status Field Layout
 * ============================================================
 */

/* Controller Status */
#define STATUS_CONTROLLER_STATUS_OFFSET    0U
#define STATUS_CONTROLLER_STATUS_SIZE      4U

/* Analog Input */
#define STATUS_ANALOG_INPUT_OFFSET         4U
#define STATUS_ANALOG_INPUT_SIZE           16U
#define STATUS_ANALOG_INPUT_CHANNELS       8U
#define STATUS_ANALOG_INPUT_CHANNEL_SIZE   2U

/* Main Stages Positions */
#define STATUS_STAGE_POSITION_OFFSET       20U
#define STATUS_STAGE_POSITION_SIZE         48U
#define STATUS_STAGE_AXIS_COUNT            6U
#define STATUS_STAGE_AXIS_SIZE             8U


/*
 * ============================================================
 * Controller Status Bit Definition
 * ============================================================
 *
 * Controller Status is a 32-bit UINT32.
 *
 * HEX representation uses Big-Endian / MSB first.
 */

#define STATUS_CONTROLLER_CONNECT_BIT      0U
#define STATUS_CONTROLLER_VOLTAGEON_BIT    1U
#define STATUS_CONTROLLER_RESERVED_BIT     2U
#define STATUS_CONTROLLER_ISMOVING_BIT     3U
#define STATUS_CONTROLLER_ISFA_BIT         4U
#define STATUS_CONTROLLER_HOMINGEND_BIT    5U
#define STATUS_CONTROLLER_ERROR_BIT        6U


/*
 * ============================================================
 * Analog Input Channel Definition
 * ============================================================
 */

#define STATUS_AI00_INDEX                  0U
#define STATUS_AI01_INDEX                  1U
#define STATUS_AI02_INDEX                  2U
#define STATUS_AI03_INDEX                  3U
#define STATUS_AI04_INDEX                  4U
#define STATUS_AI05_INDEX                  5U
#define STATUS_AI06_INDEX                  6U
#define STATUS_AI07_INDEX                  7U


/*
 * ============================================================
 * Main Stage Axis Definition
 * ============================================================
 *
 * 6 axes, each encoded as IEEE 754 DOUBLE (8 bytes).
 *
 *   X, Y, Z, RX, RY, RZ
 */

#define STATUS_AXIS_X_INDEX                0U
#define STATUS_AXIS_Y_INDEX                1U
#define STATUS_AXIS_Z_INDEX                2U
#define STATUS_AXIS_RX_INDEX               3U
#define STATUS_AXIS_RY_INDEX               4U
#define STATUS_AXIS_RZ_INDEX               5U


/*
 * ============================================================
 * Status Stream Transport Packet
 * ============================================================
 *
 * The 139-byte Routine Packet is placed into the Payload.
 *
 *   Header
 *   +----------------------+
 *   | Magic       2 bytes  |
 *   | Version     1 byte   |
 *   | Type        1 byte   |
 *   | Sequence    4 bytes  |
 *   | Payload Len 2 bytes  |
 *   +----------------------+
 *
 *   Payload       139 bytes
 *
 *   CRC16          2 bytes
 *
 *   Total packet = 151 bytes
 */

/* Packet magic: ASCII "GS" */
#define STATUS_STREAM_MAGIC0              0x47U
#define STATUS_STREAM_MAGIC1              0x53U

/* Protocol version */
#define STATUS_STREAM_VERSION              0x01U

/* Status Stream packet type */
#define STATUS_STREAM_TYPE_STATUS          0x01U

/* Transport packet header size */
#define STATUS_STREAM_HEADER_SIZE          10U

/* Transport packet Payload size */
#define STATUS_STREAM_PAYLOAD_SIZE         STATUS_ROUTINE_PACKET_SIZE

/* CRC16 size */
#define STATUS_STREAM_CRC_SIZE             2U

/* Complete Transport Packet size */
#define STATUS_STREAM_PACKET_SIZE          \
    (STATUS_STREAM_HEADER_SIZE +          \
     STATUS_STREAM_PAYLOAD_SIZE +         \
     STATUS_STREAM_CRC_SIZE)


/*
 * ============================================================
 * Status Stream Packet Header Offsets
 * ============================================================
 *
 * These offsets are also intended for the future PC Client.
 */

#define STATUS_STREAM_MAGIC0_OFFSET        0U
#define STATUS_STREAM_MAGIC1_OFFSET        1U
#define STATUS_STREAM_VERSION_OFFSET       2U
#define STATUS_STREAM_TYPE_OFFSET          3U
#define STATUS_STREAM_SEQUENCE_OFFSET      4U
#define STATUS_STREAM_LENGTH_OFFSET        8U
#define STATUS_STREAM_PAYLOAD_OFFSET       10U


/*
 * ============================================================
 * Status Stream Packet Rules
 * ============================================================
 *
 * 1. Status packet is transmitted every 200 ms.
 * 2. Payload contains one complete 139-byte Routine Packet.
 * 3. Sequence number identifies each Status packet.
 * 4. No retransmission mechanism is used.
 * 5. Invalid/incomplete/timeout packets are discarded by Client.
 * 6. Client waits for the next Status packet.
 */

/* Status TCP Server API */
void StatusTCPServer_Init(void);
void StatusTCPServer_Process(void);

bool StatusTCPServer_BuildPayload(
    uint8_t index,
    StatusDataType type,
    const uint8_t *data);

#endif /* STATUS_TCP_SERVER_H */
