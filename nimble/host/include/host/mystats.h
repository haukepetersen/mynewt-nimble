



#ifndef H_MYSTATS_
#define H_MYSTATS_


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned tx_all;            /* every adv (GAP) packet sent */
    unsigned tx_mesh_data;      /* every mesh data pkt sent */
    unsigned tx_mesh_prov;      /* every mesh prov pkt sent (adv bearer) */
    unsigned tx_mesh_beacon;

    unsigned tx_mesh_net_send;

    unsigned tx_mesh_transport_unseg;
    unsigned tx_mesh_transport_seg;
    unsigned tx_mesh_transport_ctl;
    unsigned tx_mesh_transport;

    unsigned tx_mesh_model_send;
    unsigned tx_mesh_model_pub;
    unsigned tx_mesh_model_retrans;

    unsigned rx_all;            /* every single adv (GAP) packet received */
    unsigned rx_nonmesh;        /* non BT Mesh advertising packets */
    unsigned rx_nonconn_ind;    /* non-connectable indication */

    unsigned rx_cand;
    unsigned rx_cand_nolen;
    unsigned rx_cand_malformed;

    unsigned rx_mesh_data;      /* ever mesh data packet received */
    unsigned rx_mesh_prov;      /* mesh prov packet received (adv bearer) */
    unsigned rx_mesh_beacon;
    unsigned rx_type_nomesh;

    unsigned rx_mesh_dropped_notprov;
} mystats_t;

extern mystats_t mystats;

void mystats_clear(void);

void mystats_get(mystats_t *out);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif
