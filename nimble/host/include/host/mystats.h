
/**
 *
 *
 * stats:
 *
 */


#ifndef H_MYSTATS_
#define H_MYSTATS_

#include <stdio.h>

#include "xtimer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned tx_all;            /* every adv (GAP) packet sent */

    unsigned tx_mesh_adv_put;   /* adv added to queue */
    unsigned tx_mesh_adv_send;
    unsigned tx_mesh_adv_start;
    unsigned tx_mesh_adv_stop;

    unsigned tx_mesh_net_send;
    unsigned tx_mesh_net_send_local;
    unsigned tx_mesh_net_send_adv;
    unsigned tx_mesh_net_resend;
    unsigned tx_mesh_net_relay;

    unsigned tx_mesh_transport_unseg;
    unsigned tx_mesh_transport_seg;
    unsigned tx_mesh_transport_ctl;
    unsigned tx_mesh_transport;

    unsigned tx_mesh_model_send;
    unsigned tx_mesh_model_pub;
    unsigned tx_mesh_model_retrans;

    unsigned tx_app;

    unsigned rx_all;            /* every single adv (GAP) packet received */
    unsigned rx_nonmesh;        /* non BT Mesh advertising packets */
    unsigned rx_nonconn_ind;    /* non-connectable indication */

    unsigned rx_cand;
    unsigned rx_cand_nolen;
    unsigned rx_cand_malformed;
    unsigned rx_type_nomesh;

    unsigned rx_mesh_adv_beacon;
    unsigned rx_mesh_adv_prov;      /* mesh prov packet received (adv bearer) */
    unsigned rx_mesh_adv_in;      /* ever mesh data packet received */

    unsigned rx_mesh_net;
    unsigned rx_mesh_net_notprov;
    unsigned rx_mesh_net_decode_fail;
    unsigned rx_mesh_net_relay;

    unsigned rx_mesh_net_decode_obffail;       /* unable to de-obfuscate */
    unsigned rx_mesh_net_decode_duplcache;     /* duplicate found in cache */
    unsigned rx_mesh_net_decode_ignore;        /* non-unicast src address */
    unsigned rx_mesh_net_decode_decrypt_ok;

    unsigned rx_mesh_trans;

    unsigned rx_app;

} mystats_t;

extern mystats_t mystats;

void mystats_clear(void);

void mystats_get(mystats_t *out);

void mystats_dump(void);

static inline void _time(void)
{
    printf("%lu;", (unsigned long)xtimer_now_usec64());
}

static inline void mystats_inc_tx_app(const char *op, unsigned tid) {
    ++mystats.tx_app;
    _time();
    printf("tx;app;%s;%u\n", op, tid);
}

static inline void mystats_inc_rx_app(const char *op, unsigned tid) {
    ++mystats.rx_app;
    _time();
    printf("rx;app;%s;%u\n", op, tid);
}

static inline void mystats_inc_tx_trans(void)
{
    ++mystats.tx_mesh_transport;
    _time();
    printf("tx;trans\n");
}

static inline void mystats_inc_rx_trans(void)
{
    ++mystats.rx_mesh_trans;
    _time();
    printf("rx;trans\n");
}

static inline void mystats_inc_tx_net_send_adv(void)
{
    ++mystats.tx_mesh_net_send_adv;
    _time();
    printf("tx;net_send_adv\n");
}

static inline void mystats_inc_tx_net_send_local(void)
{
    ++mystats.tx_mesh_net_send_local;
    _time();
    printf("tx;net_send_local\n");
}

static inline void mystats_inc_tx_net_send_relay(void)
{
    ++mystats.tx_mesh_net_relay;
    _time();
    printf("tx;net_send_relay\n");
}

// static inline void mystats_inc_rx_net(void)
// {
//     ++mystats.rx_mesh_net;
//     _time();
//     printf("rx:net\n");
// }

static inline void mystats_inc_rx_net_drop(void)
{
    ++mystats.rx_mesh_net_decode_fail;
    _time();
    printf("rx:net_drop\n");
}

static inline void mystats_inc_tx_adv_send(void)
{
    ++mystats.tx_mesh_adv_send;
    _time();
    printf("tx;adv_send\n");
}

static inline void mystats_inc_rx_adv_data(void)
{
    ++mystats.rx_mesh_adv_in;
    _time();
    printf("rx;adv_data\n");
}

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif
