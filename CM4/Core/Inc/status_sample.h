#ifndef STATUS_SAMPLE_H
#define STATUS_SAMPLE_H

#include <stdint.h>

/*
 * ============================================================
 * Status Sample Data
 * ============================================================
 *
 * Five simulated Routine Status data sets.
 *
 * Each sample contains exactly 68 bytes:
 *
 *   Bytes 00 ~ 03 : Controller Status   UINT32
 *   Bytes 04 ~ 19 : Analog Input        UINT16 x 8
 *   Bytes 20 ~ 67 : Stage Position      DOUBLE x 6
 *
 * Multi-byte values use Big-Endian / MSB first.
 */

#define STATUS_SAMPLE_COUNT              5U
#define STATUS_SAMPLE_RAW_SIZE           68U
#define STATUS_SAMPLE_PACKET_SIZE        139U


/*
 * ============================================================
 * Controller Status Bit Definition
 * ============================================================
 *
 * Bit 0 : CONNECT
 * Bit 1 : VOLTAGEON
 * Bit 3 : ISMOVING
 * Bit 4 : ISFA
 * Bit 5 : HOMINGEND
 *
 * Bit 2 and Bit 6 are not used by the sample data.
 */


/*
 * ============================================================
 * Five simulated raw status data sets
 * ============================================================
 *
 * Sample 0:
 *   CONNECT=1
 *   VOLTAGEON=1
 *   ISMOVING=0
 *   ISFA=0
 *   HOMINGEND=0
 *
 * Sample 1:
 *   CONNECT=1
 *   VOLTAGEON=1
 *   ISMOVING=1
 *   ISFA=0
 *   HOMINGEND=0
 *
 * Sample 2:
 *   CONNECT=1
 *   VOLTAGEON=1
 *   ISMOVING=0
 *   ISFA=1
 *   HOMINGEND=0
 *
 * Sample 3:
 *   CONNECT=1
 *   VOLTAGEON=1
 *   ISMOVING=1
 *   ISFA=0
 *   HOMINGEND=1
 *
 * Sample 4:
 *   CONNECT=1
 *   VOLTAGEON=1
 *   ISMOVING=0
 *   ISFA=0
 *   HOMINGEND=1
 */

static const uint8_t status_raw_data_buffers[STATUS_SAMPLE_COUNT]
                                                   [STATUS_SAMPLE_RAW_SIZE] =
{
    /* --------------------------------------------------------
     * Sample 0
     * -------------------------------------------------------- */
    {
        /* Controller Status = 0x00000003 */
        0x00U, 0x00U, 0x00U, 0x03U,

        /* AI00 ~ AI07 */
        0x40U, 0x00U,  /* AI00 = 16384 -> -5.0 V */
        0x00U, 0x00U,  /* AI01 = Reserved */
        0x19U, 0x9AU,  /* AI02 = 6554  -> 1.0 V */
        0x33U, 0x33U,  /* AI03 = 13107 -> 2.0 V */
        0x4CU, 0xCCU,  /* AI04 = 19660 -> 3.0 V */
        0x66U, 0x66U,  /* AI05 = 26214 -> 4.0 V */
        0x80U, 0x00U,  /* AI06 = 32768 -> 5.0 V */
        0x99U, 0x99U,  /* AI07 = 39321 -> 6.0 V */

        /* X, Y, Z, RX, RY, RZ */
        0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U
    },

    /* --------------------------------------------------------
     * Sample 1
     * -------------------------------------------------------- */
    {
        /* Controller Status = 0x0000000B */
        0x00U, 0x00U, 0x00U, 0x0BU,

        /* AI00 ~ AI07 */
        0x60U, 0x00U,  /* AI00 = 24576 -> -2.5 V */
        0x00U, 0x00U,  /* AI01 = Reserved */
        0x33U, 0x33U,  /* AI02 = 2.0 V */
        0x4CU, 0xCCU,  /* AI03 = 3.0 V */
        0x66U, 0x66U,  /* AI04 = 4.0 V */
        0x80U, 0x00U,  /* AI05 = 5.0 V */
        0x99U, 0x99U,  /* AI06 = 6.0 V */
        0xB3U, 0x32U,  /* AI07 = 7.0 V */

        /* X=10, Y=20, Z=30, RX=1, RY=2, RZ=3 */
        0x40U, 0x24U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x40U, 0x34U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x40U, 0x3EU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x3FU, 0xF0U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x40U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x40U, 0x08U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U
    },

    /* --------------------------------------------------------
     * Sample 2
     * -------------------------------------------------------- */
    {
        /* Controller Status = 0x00000013 */
        0x00U, 0x00U, 0x00U, 0x13U,

        /* AI00 ~ AI07 */
        0x80U, 0x00U,  /* AI00 = 32768 -> 0.0 V */
        0x00U, 0x00U,  /* AI01 = Reserved */
        0x4CU, 0xCCU,  /* AI02 = 3.0 V */
        0x66U, 0x66U,  /* AI03 = 4.0 V */
        0x80U, 0x00U,  /* AI04 = 5.0 V */
        0x99U, 0x99U,  /* AI05 = 6.0 V */
        0xB3U, 0x32U,  /* AI06 = 7.0 V */
        0xCCU, 0xCCU,  /* AI07 = 8.0 V */

        /* X=20, Y=10, Z=5, RX=2, RY=4, RZ=6 */
        0x40U, 0x34U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x40U, 0x24U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x40U, 0x14U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x40U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x40U, 0x10U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x40U, 0x18U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U
    },

    /* --------------------------------------------------------
     * Sample 3
     * -------------------------------------------------------- */
    {
        /* Controller Status = 0x0000002B */
        0x00U, 0x00U, 0x00U, 0x2BU,

        /* AI00 ~ AI07 */
        0xBFU, 0xFFU,  /* AI00 = 49151 -> 5.0 V */
        0x00U, 0x00U,  /* AI01 = Reserved */
        0x66U, 0x66U,  /* AI02 = 4.0 V */
        0x80U, 0x00U,  /* AI03 = 5.0 V */
        0x99U, 0x99U,  /* AI04 = 6.0 V */
        0xB3U, 0x32U,  /* AI05 = 7.0 V */
        0xCCU, 0xCCU,  /* AI06 = 8.0 V */
        0xE6U, 0x66U,  /* AI07 = 9.0 V */

        /* X=30, Y=15, Z=10, RX=3, RY=6, RZ=9 */
        0x40U, 0x3EU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x40U, 0x2EU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x40U, 0x24U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x40U, 0x08U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x40U, 0x18U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x40U, 0x22U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U
    },

    /* --------------------------------------------------------
     * Sample 4
     * -------------------------------------------------------- */
    {
        /* Controller Status = 0x00000023 */
        0x00U, 0x00U, 0x00U, 0x23U,

        /* AI00 ~ AI07 */
        0xE6U, 0x66U,  /* AI00 = 58982 -> 8.0 V */
        0x00U, 0x00U,  /* AI01 = Reserved */
        0x80U, 0x00U,  /* AI02 = 5.0 V */
        0x99U, 0x99U,  /* AI03 = 6.0 V */
        0xB3U, 0x32U,  /* AI04 = 7.0 V */
        0xCCU, 0xCCU,  /* AI05 = 8.0 V */
        0xE6U, 0x66U,  /* AI06 = 9.0 V */
        0xFFU, 0xFFU,  /* AI07 = 10.0 V */

        /* X=40, Y=20, Z=15, RX=4, RY=8, RZ=12 */
        0x40U, 0x44U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x40U, 0x34U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x40U, 0x2EU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x40U, 0x10U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x40U, 0x20U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x40U, 0x28U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U
    }
};


/*
 * ============================================================
 * Runtime Routine Packet Buffers
 * ============================================================
 *
 * Each buffer will contain:
 *
 *   '>' + 136 HEX characters + "\r\n"
 *
 * Total = 139 bytes.
 *
 * These buffers are filled by STM32 at runtime after the
 * corresponding raw sample is converted to HEX.
 */

extern uint8_t status_payload_buffers[STATUS_SAMPLE_COUNT]
                                      [STATUS_SAMPLE_PACKET_SIZE];

#endif /* STATUS_SAMPLE_H */ 