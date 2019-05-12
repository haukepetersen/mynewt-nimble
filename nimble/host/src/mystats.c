
#include <stdio.h>

#include "string.h"
#include "host/mystats.h"

mystats_t mystats = { 0 };

mutex_t mystats_lock = MUTEX_INIT;

int mystats_en = 0;

void mystats_clear(void)
{
    memset(&mystats, 0, sizeof(mystats));
}

void mystats_get(mystats_t *out)
{
    memcpy(out, &mystats, sizeof(mystats));
}

void mystats_dump(void)
{
    mystats_t stats;
    mystats_get(&stats);

    // printf("Stats:\n");
    printf("TX_ll;%u\n", stats.tx_all);

    printf("TX_mesh_adv_put;%u\n", stats.tx_mesh_adv_put);
    printf("TX_mesh_adv_send;%u\n", stats.tx_mesh_adv_send);
    printf("TX_mesh_adv_start;%u\n", stats.tx_mesh_adv_start);
    printf("TX_mesh_adv_stop;%u\n", stats.tx_mesh_adv_stop);

    printf("TX_mesh_net_send;%u\n", stats.tx_mesh_net_send);
    printf("TX_mesh_net_send_local;%u\n", stats.tx_mesh_net_send_local);
    printf("TX_mesh_net_send_adv;%u\n", stats.tx_mesh_net_send_adv);
    printf("TX_mesh_net_send_resend;%u\n", stats.tx_mesh_net_resend);
    printf("TX_mesh_net_send_relay;%u\n", stats.tx_mesh_net_relay);

    printf("TX_mesh_transport_unseg;%u\n", stats.tx_mesh_transport_unseg);
    printf("TX_mesh_transport_seg;%u\n", stats.tx_mesh_transport_seg);
    printf("TX_mesh_transport_ctl;%u\n", stats.tx_mesh_transport_ctl);
    printf("TX_mesh_transport;%u\n", stats.tx_mesh_transport);

    printf("TX_mesh_model_send;%u\n", stats.tx_mesh_model_send);
    printf("TX_mesh_model_pub;%u\n", stats.tx_mesh_model_pub);
    printf("TX_mesh_model_retrans;%u\n", stats.tx_mesh_model_retrans);

    printf("TX_app;%u\n", stats.tx_app);

    // puts("--- RX ---");
    printf("RX_ll;%u\n", stats.rx_all);
    printf("RX_non_mesh;%u\n", stats.rx_nonmesh);
    printf("RX_nonconn_IND;%u\n", stats.rx_nonconn_ind);

    // printf("RX cand;%u\n", stats.rx_cand);
    printf("RX_mesh_nomesh;%u\n", stats.rx_type_nomesh);

    printf("RX_mesh_adv_data;%u\n", stats.rx_mesh_adv_in);
    printf("RX_mesh_adv_prov;%u\n", stats.rx_mesh_adv_prov);
    printf("RX_mesh_adv_beacon;%u\n", stats.rx_mesh_adv_beacon);

    printf("RX_mesh_net;%u\n", stats.rx_mesh_net);
    printf("RX_mesh_net_drop_notprov;%u\n", stats.rx_mesh_net_notprov);
    printf("RX_mesh_net_drop_decode_fail;%u\n", stats.rx_mesh_net_decode_fail);
    // printf("RX mesh net pass to trans;%u\n", stats.rx_mesh_net_pass_to_trans);
    printf("RX_mesh_net_will_relay;%u\n", stats.rx_mesh_net_relay);
    // printf("RX mesh net dec err: obfus;%u\n", stats.rx_mesh_net_decode_obffail);
    // printf("RX mesh net dec err: cached;%u\n", stats.rx_mesh_net_decode_duplcache);
    // printf("RX mesh net dec err: ignored;%u\n", stats.rx_mesh_net_decode_ignore);
    printf("RX_mesh_net_decode_ok;%u\n", stats.rx_mesh_net_decode_decrypt_ok);

    printf("RX_mesh_trans;%u\n", stats.rx_mesh_trans);

    printf("RX_app;%u\n", stats.rx_app);
}
