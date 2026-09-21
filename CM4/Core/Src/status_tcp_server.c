#include "main.h"
#include "status_tcp_server.h"

#include "lwip/tcp.h"
#include "lwip/ip_addr.h"

#include <string.h>

#include "status_sample.h"

static struct tcp_pcb *status_server_pcb = NULL;
static struct tcp_pcb *status_client_pcb = NULL;

static uint32_t status_last_send_tick = 0U;

uint8_t status_payload_buffers[STATUS_PAYLOAD_BUFFER_COUNT]
                              [STATUS_ROUTINE_PACKET_SIZE];

static uint8_t status_packet_buffer[STATUS_STREAM_PACKET_SIZE];
static uint32_t status_sequence = 0U;

static err_t StatusTCPServer_Accept(void *arg,
                                    struct tcp_pcb *newpcb,
                                    err_t err);

static void StatusTCPServer_Connected(void *arg,
                                      err_t err);

static void StatusTCPServer_CloseClient(void);



static uint16_t StatusTCPServer_CalculateCRC16(
    const uint8_t *data,
    uint32_t length)
{
    uint16_t crc = 0xFFFFU;

    for (uint32_t i = 0U; i < length; i++)
    {
        crc ^= data[i];

        for (uint32_t bit = 0U; bit < 8U; bit++)
        {
            if ((crc & 0x0001U) != 0U)
            {
                crc = (uint16_t)((crc >> 1U) ^ 0xA001U);
            }
            else
            {
                crc >>= 1U;
            }
        }
    }

    return crc;
}


/*
 * ============================================================
 * Routine Status Payload Builder
 * ============================================================
 *
 * Validation version:
 *
 *   status_sample.h provides five simulated 68-byte raw
 *   status data sets. Each sample is converted into its own
 *   139-byte payload buffer for Status Stream validation.
 *
 *   Sample 0 -> status_payload_buffers[0][139]
 *   Sample 1 -> status_payload_buffers[1][139]
 *   Sample 2 -> status_payload_buffers[2][139]
 *   Sample 3 -> status_payload_buffers[3][139]
 *   Sample 4 -> status_payload_buffers[4][139]
 *
 * The five samples are used only for validation so the PC
 * Client can clearly display changing status values.
 *
 * After validation, status_sample.h will be removed.
 * The production version will use one 68-byte raw data
 * buffer and one 139-byte payload buffer.
 *
 * Payload format:
 *
 *   '>' + 136 ASCII HEX characters + "\r\n"
 *
 * The API currently supports:
 *
 *   STATUS_DATA_RAW_68
 *   STATUS_DATA_HEX_136
 */
bool StatusTCPServer_BuildPayload(
    uint8_t index,
    StatusDataType type,
    const uint8_t *data)
{
    static const char hex_table[] = "0123456789ABCDEF";
    uint32_t i;

    if ((index >= STATUS_PAYLOAD_BUFFER_COUNT) ||
        (data == NULL))
    {
        return false;
    }

    status_payload_buffers[index][0] = '>';

    if (type == STATUS_DATA_RAW_68)
    {
        for (i = 0U; i < STATUS_ROUTINE_RAW_SIZE; i++)
        {
            uint8_t value = data[i];

            status_payload_buffers[index][1U + (i * 2U)] =
                hex_table[(value >> 4U) & 0x0FU];

            status_payload_buffers[index][2U + (i * 2U)] =
                hex_table[value & 0x0FU];
        }
    }
    else if (type == STATUS_DATA_HEX_136)
    {
        memcpy(&status_payload_buffers[index][1],
               data,
               STATUS_ROUTINE_HEX_SIZE);
    }
    else
    {
        return false;
    }

    status_payload_buffers[index][137] = '\r';
    status_payload_buffers[index][138] = '\n';

    return true;
}


static bool StatusTCPServer_BuildPacket(
    uint8_t payload_index,
    uint32_t sequence)
{
    uint16_t crc;

    if (payload_index >= STATUS_PAYLOAD_BUFFER_COUNT)
    {
        return false;
    }

    status_packet_buffer[STATUS_STREAM_MAGIC0_OFFSET] =
        STATUS_STREAM_MAGIC0;

    status_packet_buffer[STATUS_STREAM_MAGIC1_OFFSET] =
        STATUS_STREAM_MAGIC1;

    status_packet_buffer[STATUS_STREAM_VERSION_OFFSET] =
        STATUS_STREAM_VERSION;

    status_packet_buffer[STATUS_STREAM_TYPE_OFFSET] =
        STATUS_STREAM_TYPE_STATUS;

    status_packet_buffer[STATUS_STREAM_SEQUENCE_OFFSET + 0U] =
        (uint8_t)(sequence & 0xFFU);

    status_packet_buffer[STATUS_STREAM_SEQUENCE_OFFSET + 1U] =
        (uint8_t)((sequence >> 8U) & 0xFFU);

    status_packet_buffer[STATUS_STREAM_SEQUENCE_OFFSET + 2U] =
        (uint8_t)((sequence >> 16U) & 0xFFU);

    status_packet_buffer[STATUS_STREAM_SEQUENCE_OFFSET + 3U] =
        (uint8_t)((sequence >> 24U) & 0xFFU);

    status_packet_buffer[STATUS_STREAM_LENGTH_OFFSET + 0U] =
        (uint8_t)(STATUS_STREAM_PAYLOAD_SIZE & 0xFFU);

    status_packet_buffer[STATUS_STREAM_LENGTH_OFFSET + 1U] =
        (uint8_t)((STATUS_STREAM_PAYLOAD_SIZE >> 8U) & 0xFFU);

    memcpy(&status_packet_buffer[STATUS_STREAM_PAYLOAD_OFFSET],
           status_payload_buffers[payload_index],
           STATUS_STREAM_PAYLOAD_SIZE);

    crc = StatusTCPServer_CalculateCRC16(
        status_packet_buffer,
        STATUS_STREAM_PAYLOAD_OFFSET + STATUS_STREAM_PAYLOAD_SIZE);

    status_packet_buffer[STATUS_STREAM_PACKET_SIZE - 2U] =
        (uint8_t)(crc & 0xFFU);

    status_packet_buffer[STATUS_STREAM_PACKET_SIZE - 1U] =
        (uint8_t)((crc >> 8U) & 0xFFU);

    return true;
}


void StatusTCPServer_Init(void)
{
    err_t err;
    struct tcp_pcb *pcb;

    pcb = tcp_new();

    if (pcb == NULL)
    {
        return;
    }

    err = tcp_bind(pcb, IP_ADDR_ANY, STATUS_TCP_SERVER_PORT);

    if (err != ERR_OK)
    {
        tcp_close(pcb);
        return;
    }

    status_server_pcb = tcp_listen(pcb);

    if (status_server_pcb == NULL)
    {
        tcp_close(pcb);
        return;
    }

    tcp_accept(status_server_pcb, StatusTCPServer_Accept);
}


void StatusTCPServer_Process(void)
{
    static uint8_t sample_index = 0U;

    if (status_client_pcb == NULL)
    {
        return;
    }

    if ((HAL_GetTick() - status_last_send_tick) < STATUS_TCP_PERIOD_MS)
    {
        return;
    }

    status_last_send_tick = HAL_GetTick();


    if (StatusTCPServer_BuildPayload(sample_index,
                                     STATUS_DATA_RAW_68,
                                     status_raw_data_buffers[sample_index]))
    {
        printf("[STATUS] Sample %u BuildPayload OK\r\n",
               sample_index);

        printf("[STATUS] Payload[0..20]: ");

        for (uint32_t i = 0U; i <= 20U; i++)
        {
            printf("%c", status_payload_buffers[sample_index][i]);
        }

        printf("\r\n");


        if (StatusTCPServer_BuildPacket(sample_index,
                                        status_sequence))
        {
            uint16_t packet_crc =
                (uint16_t)status_packet_buffer[STATUS_STREAM_PACKET_SIZE - 2U] |
                ((uint16_t)status_packet_buffer[STATUS_STREAM_PACKET_SIZE - 1U] << 8U);

            printf("[STATUS] Packet Build OK\r\n");
            printf("[STATUS] Packet Length = %u\r\n",
                STATUS_STREAM_PACKET_SIZE);
            printf("[STATUS] Sequence = %lu\r\n",
                (unsigned long)status_sequence);
            printf("[STATUS] Payload Length = %u\r\n",
                STATUS_STREAM_PAYLOAD_SIZE);
            printf("[STATUS] CRC = %04X\r\n",
                packet_crc);

            printf("[STATUS] Header: ");

            for (uint32_t i = 0U;
                i < STATUS_STREAM_HEADER_SIZE;
                i++)
            {
                printf("%02X ", status_packet_buffer[i]);
            }

            printf("\r\n");

            status_sequence++;
        }
        else
        {
            printf("[STATUS] Packet Build FAILED\r\n");
        }

    }
    else
    {
        printf("[STATUS] Sample %u BuildPayload FAILED\r\n",
               sample_index);
    }

    if (tcp_sndbuf(status_client_pcb) < STATUS_STREAM_PACKET_SIZE)
    {
        return;
    }

    if (tcp_write(status_client_pcb,
                status_payload_buffers[sample_index],
                STATUS_STREAM_PACKET_SIZE,
                TCP_WRITE_FLAG_COPY) != ERR_OK)
    {
        printf("[STATUS] Sample %u TCP write FAILED\r\n", sample_index);

        StatusTCPServer_CloseClient();
        return;
    }

    tcp_output(status_client_pcb);

    printf("[STATUS] Sample %u TCP send OK, %u bytes\r\n", sample_index, STATUS_STREAM_PACKET_SIZE);

    sample_index++;

    if (sample_index >= STATUS_SAMPLE_COUNT)
    {
        sample_index = 0U;
    }

}

static err_t StatusTCPServer_Accept(void *arg,
                                    struct tcp_pcb *newpcb,
                                    err_t err)
{
    if ((err != ERR_OK) || (newpcb == NULL))
    {
        return ERR_VAL;
    }

    if (status_client_pcb != NULL)
    {
        tcp_abort(newpcb);
        return ERR_ABRT;
    }

    status_client_pcb = newpcb;

    status_last_send_tick = HAL_GetTick();

    tcp_arg(newpcb, NULL);
    tcp_err(newpcb, StatusTCPServer_Connected);

    return ERR_OK;
}

static void StatusTCPServer_Connected(void *arg,
                                      err_t err)
{
    status_client_pcb = NULL;
}

static void StatusTCPServer_CloseClient(void)
{
    struct tcp_pcb *pcb = status_client_pcb;

    status_client_pcb = NULL;

    if (pcb == NULL)
    {
        return;
    }

    tcp_close(pcb);
}