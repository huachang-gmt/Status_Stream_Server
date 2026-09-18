#include "main.h"
#include "status_tcp_server.h"

#include "lwip/tcp.h"
#include "lwip/ip_addr.h"

#include <string.h>

#define STATUS_TCP_SERVER_PORT       8888U
#define STATUS_TCP_PERIOD_MS         200U

static struct tcp_pcb *status_server_pcb = NULL;
static struct tcp_pcb *status_client_pcb = NULL;

static uint32_t status_last_send_tick = 0U;

static err_t StatusTCPServer_Accept(void *arg,
                                    struct tcp_pcb *newpcb,
                                    err_t err);

static void StatusTCPServer_Connected(void *arg,
                                      err_t err);

static void StatusTCPServer_CloseClient(void);

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
    const char test_data[] = "STATUS_TEST\r\n";

    if (status_client_pcb == NULL)
    {
        return;
    }

    if ((HAL_GetTick() - status_last_send_tick) < STATUS_TCP_PERIOD_MS)
    {
        return;
    }

    status_last_send_tick = HAL_GetTick();

    if (tcp_sndbuf(status_client_pcb) < sizeof(test_data) - 1U)
    {
        return;
    }

    if (tcp_write(status_client_pcb,
                  test_data,
                  sizeof(test_data) - 1U,
                  TCP_WRITE_FLAG_COPY) != ERR_OK)
    {
        StatusTCPServer_CloseClient();
        return;
    }

    tcp_output(status_client_pcb);
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