
#include <stdio.h>

#include "string.h"
#include "host/mystats.h"

mystats_t mystats = { 0 };

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

    printf("Stats:\n");
    printf("TX ADV\t\t\t\t%u\n", stats.tx_all);

    printf("TX mesh adv put\t\t%u\n", stats.tx_mesh_adv_put);
    printf("TX mesh adv send\t\t%u\n", stats.tx_mesh_adv_send);
    printf("TX mesh adv start\t\t%u\n", stats.tx_mesh_adv_start);
    printf("TX mesh adv stop\t\t%u\n", stats.tx_mesh_adv_stop);

    printf("TX mesh net send\t\t%u\n", stats.tx_mesh_net_send);
    printf("TX mesh net send local\t\t%u\n", stats.tx_mesh_net_send_local);
    printf("TX mesh net send adv\t\t%u\n", stats.tx_mesh_net_send_adv);
    printf("TX mesh net send resend\t%u\n", stats.tx_mesh_net_resend);
    printf("TX mesh net send relay\t\t%u\n", stats.tx_mesh_net_relay);

    printf("TX mesh transport unseg\t%u\n", stats.tx_mesh_transport_unseg);
    printf("TX mesh transport seg\t\t%u\n", stats.tx_mesh_transport_seg);
    printf("TX mesh transport ctl\t\t%u\n", stats.tx_mesh_transport_ctl);
    printf("TX mesh transport\t\t%u\n", stats.tx_mesh_transport);

    printf("TX mesh model send\t\t%u\n", stats.tx_mesh_model_send);
    printf("TX mesh model pub\t\t%u\n", stats.tx_mesh_model_pub);
    printf("TX mesh model retrans\t\t%u\n", stats.tx_mesh_model_retrans);

    printf("TX app\t\t\t\t%u\n", stats.tx_app);

    puts("--- RX ---");
    printf("RX ADV\t\t\t\t%u\n", stats.rx_all);
    printf("RX non mesh\t\t\t%u\n", stats.rx_nonmesh);
    printf("RX nonconn IND\t\t\t%u\n", stats.rx_nonconn_ind);

    printf("RX cand\t\t\t%u\n", stats.rx_cand);
    printf("RX mesh nomesh\t\t\t%u\n", stats.rx_type_nomesh);

    printf("RX mesh adv data\t\t%u\n", stats.rx_mesh_adv_in);
    printf("RX mesh adv prov\t\t%u\n", stats.rx_mesh_adv_prov);
    printf("RX mesh adv beacon\t\t%u\n", stats.rx_mesh_adv_beacon);

    printf("RX mesh net\t\t\t%u\n", stats.rx_mesh_net);
    printf("RX mesh net drop notprov\t%u\n", stats.rx_mesh_net_notprov);
    printf("RX mesh net drop decode fail\t%u\n", stats.rx_mesh_net_decode_fail);
    // printf("RX mesh net pass to trans\t%u\n", stats.rx_mesh_net_pass_to_trans);
    printf("RX mesh net will relay\t\t%u\n", stats.rx_mesh_net_relay);
    printf("RX mesh net dec err: obfus\t%u\n", stats.rx_mesh_net_decode_obffail);
    printf("RX mesh net dec err: cached\t%u\n", stats.rx_mesh_net_decode_duplcache);
    printf("RX mesh net dec err: ignored\t%u\n", stats.rx_mesh_net_decode_ignore);
    printf("RX mesh net decode ok\t\t%u\n", stats.rx_mesh_net_decode_decrypt_ok);

    printf("RX app:\t\t\t%u\n", stats.rx_app);
}
