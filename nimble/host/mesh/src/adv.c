/*  Bluetooth Mesh */

/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mesh/mesh.h"

#include "syscfg/syscfg.h"
#define BT_DBG_ENABLED (MYNEWT_VAL(BLE_MESH_DEBUG_ADV))
#include "host/ble_hs_log.h"

#include "host/ble_hs_adv.h"
#include "host/ble_gap.h"
#include "nimble/hci_common.h"
#include "mesh/porting.h"

#include "adv.h"
#include "net.h"
#include "foundation.h"
#include "beacon.h"
#include "prov.h"
#include "proxy.h"

/* TMP: custom stats only */
#include "host/mystats.h"

/* Convert from ms to 0.625ms units */
#define ADV_SCAN_UNIT(_ms) ((_ms) * 8 / 5)

/* Window and Interval are equal for continuous scanning */
#define MESH_SCAN_INTERVAL_MS 30
#define MESH_SCAN_WINDOW_MS   30
#define MESH_SCAN_INTERVAL    ADV_SCAN_UNIT(MESH_SCAN_INTERVAL_MS)
#define MESH_SCAN_WINDOW      ADV_SCAN_UNIT(MESH_SCAN_WINDOW_MS)

/* Pre-5.0 controllers enforce a minimum interval of 100ms
 * whereas 5.0+ controllers can go down to 20ms.
 */
#define ADV_INT_DEFAULT_MS 100
#define ADV_INT_FAST_MS    20

static s32_t adv_int_min =  ADV_INT_DEFAULT_MS;

/* TinyCrypt PRNG consumes a lot of stack space, so we need to have
 * an increased call stack whenever it's used.
 */
#if MYNEWT
#define ADV_STACK_SIZE 768
OS_TASK_STACK_DEFINE(g_blemesh_stack, ADV_STACK_SIZE);
struct os_task adv_task;
#endif

static struct ble_npl_eventq adv_queue;
extern u8_t g_mesh_addr_type;
static int adv_initialized = false;

static os_membuf_t adv_buf_mem[OS_MEMPOOL_SIZE(
		MYNEWT_VAL(BLE_MESH_ADV_BUF_COUNT),
		BT_MESH_ADV_DATA_SIZE + BT_MESH_MBUF_HEADER_SIZE)];

struct os_mbuf_pool adv_os_mbuf_pool;
static struct os_mempool adv_buf_mempool;

static const u8_t adv_type[] = {
	[BT_MESH_ADV_PROV]   = BLE_HS_ADV_TYPE_MESH_PROV,
	[BT_MESH_ADV_DATA]   = BLE_HS_ADV_TYPE_MESH_MESSAGE,
	[BT_MESH_ADV_BEACON] = BLE_HS_ADV_TYPE_MESH_BEACON,
	[BT_MESH_ADV_URI]    = BLE_HS_ADV_TYPE_URI,
};


static struct bt_mesh_adv adv_pool[CONFIG_BT_MESH_ADV_BUF_COUNT];

static struct bt_mesh_adv *adv_alloc(int id)
{
	return &adv_pool[id];
}

static inline void adv_send_start(u16_t duration, int err,
				  const struct bt_mesh_send_cb *cb,
				  void *cb_data)
{
	if (cb && cb->start) {
		cb->start(duration, err, cb_data);
	}
}

static inline void adv_send_end(int err, const struct bt_mesh_send_cb *cb,
				void *cb_data)
{
	if (cb && cb->end) {
		cb->end(err, cb_data);
	}
}

static inline void adv_send(struct os_mbuf *buf)
{
	const struct bt_mesh_send_cb *cb = BT_MESH_ADV(buf)->cb;
	void *cb_data = BT_MESH_ADV(buf)->cb_data;
	struct ble_gap_adv_params param = { 0 };
	u16_t duration, adv_int;
	struct bt_data ad;
	int err;

	// ++mystats.tx_mesh_adv_send;
	// int mic;
	// memcpy(&mic, buf->om_data + (buf->om_len -4), 4);
	// mystats_inc_tx_adv_send(mic);
    ++mystats.tx_mesh_adv_send;

	adv_int = max(adv_int_min,
		      BT_MESH_TRANSMIT_INT(BT_MESH_ADV(buf)->xmit));
	duration = (((BT_MESH_TRANSMIT_COUNT(BT_MESH_ADV(buf)->xmit) + 1) * (adv_int)) + 3);

	// printf("int: %i, xmit-int: %i, win: %i, xmit: %i, adv_int: %i\n",
	// 		(int)adv_int_min,
	// 		(int)BT_MESH_TRANSMIT_INT(BT_MESH_ADV(buf)->xmit),
	// 		(int)MESH_SCAN_WINDOW_MS,
	// 		(int)BT_MESH_TRANSMIT_COUNT(BT_MESH_ADV(buf)->xmit), (int)adv_int);
	// printf("duration: %i\n", (int)duration);

	BT_DBG("type %u om_len %u: %s", BT_MESH_ADV(buf)->type,
	       buf->om_len, bt_hex(buf->om_data, buf->om_len));
	BT_DBG("count %u interval %ums duration %ums",
	       BT_MESH_TRANSMIT_COUNT(BT_MESH_ADV(buf)->xmit) + 1, adv_int,
	       duration);

	ad.type = adv_type[BT_MESH_ADV(buf)->type];
	ad.data_len = buf->om_len;
	ad.data = buf->om_data;

	param.itvl_min = ADV_SCAN_UNIT(adv_int);
	param.itvl_max = param.itvl_min;
	param.conn_mode = BLE_GAP_CONN_MODE_NON;

	// printf("adv: cnt: %i, itvl: %i, duration: %i\n",
	// 		(int)(BT_MESH_TRANSMIT_COUNT(BT_MESH_ADV(buf)->xmit + 1)),
	// 		(int)adv_int,
	// 		(int)duration);

	++mystats.tx_mesh_adv_start;
	err = bt_le_adv_start(&param, &ad, 1, NULL, 0);
	net_buf_unref(buf);
	adv_send_start(duration, err, cb, cb_data);
	if (err) {
		BT_ERR("Advertising failed: err %d", err);
		return;
	}

	BT_DBG("Advertising started. Sleeping %u ms", duration);

	k_sleep(K_MSEC(duration));

	++mystats.tx_mesh_adv_stop;
	err = bt_le_adv_stop(false);
	adv_send_end(err, cb, cb_data);
	if (err) {
		BT_ERR("Stopping advertising failed: err %d", err);
		return;
	}

	BT_DBG("Advertising stopped");
}

void
mesh_adv_thread(void *args)
{
	static struct ble_npl_event *ev;
	struct os_mbuf *buf;
#if (MYNEWT_VAL(BLE_MESH_PROXY))
	s32_t timeout;
#endif

	BT_DBG("started");

	while (1) {
#if (MYNEWT_VAL(BLE_MESH_PROXY))
		ev = ble_npl_eventq_get(&adv_queue, 0);
		while (!ev) {
			timeout = bt_mesh_proxy_adv_start();
			BT_DBG("Proxy Advertising up to %d ms", (int) timeout);

			// FIXME: should we redefine K_SECONDS macro instead in glue?
			if (timeout != K_FOREVER) {
				timeout = ble_npl_time_ms_to_ticks32(timeout);
			}

			ev = ble_npl_eventq_get(&adv_queue, timeout);
			bt_mesh_proxy_adv_stop();
		}
#else
		ev = ble_npl_eventq_get(&adv_queue, BLE_NPL_TIME_FOREVER);
#endif

		if (!ev || !ble_npl_event_get_arg(ev)) {
			continue;
		}

		buf = ble_npl_event_get_arg(ev);

		/* busy == 0 means this was canceled */
		if (BT_MESH_ADV(buf)->busy) {
			BT_MESH_ADV(buf)->busy = 0;
			adv_send(buf);
		}

		/* os_sched(NULL); */
	}
}

void bt_mesh_adv_update(void)
{
	static struct ble_npl_event ev = { };

	BT_DBG("");

	ble_npl_eventq_put(&adv_queue, &ev);
}

struct os_mbuf *bt_mesh_adv_create_from_pool(struct os_mbuf_pool *pool,
					     bt_mesh_adv_alloc_t get_id,
					     enum bt_mesh_adv_type type,
					     u8_t xmit, s32_t timeout)
{
	struct bt_mesh_adv *adv;
	struct os_mbuf *buf;

	if (atomic_test_bit(bt_mesh.flags, BT_MESH_SUSPENDED)) {
		BT_WARN("Refusing to allocate buffer while suspended");
		return NULL;
	}

	buf = os_mbuf_get_pkthdr(pool, BT_MESH_ADV_USER_DATA_SIZE);
	if (!buf) {
		return NULL;
	}

	adv = get_id(net_buf_id(buf));
	BT_MESH_ADV(buf) = adv;

	memset(adv, 0, sizeof(*adv));

	adv->type         = type;
	adv->xmit         = xmit;

	adv->ref_cnt = 1;
	ble_npl_event_set_arg(&adv->ev, buf);

	return buf;
}

struct os_mbuf *bt_mesh_adv_create(enum bt_mesh_adv_type type, u8_t xmit,
				   s32_t timeout)
{
	return bt_mesh_adv_create_from_pool(&adv_os_mbuf_pool, adv_alloc, type,
					    xmit, timeout);
}

void bt_mesh_adv_send(struct os_mbuf *buf, const struct bt_mesh_send_cb *cb,
		      void *cb_data)
{
	BT_DBG("buf %p, type 0x%02x len %u: %s", buf, BT_MESH_ADV(buf)->type, buf->om_len,
	       bt_hex(buf->om_data, buf->om_len));

	BT_MESH_ADV(buf)->cb = cb;
	BT_MESH_ADV(buf)->cb_data = cb_data;
	BT_MESH_ADV(buf)->busy = 1;

	net_buf_put(&adv_queue, net_buf_ref(buf));
}

static void bt_mesh_scan_cb(const bt_addr_le_t *addr, s8_t rssi,
			    u8_t adv_type, struct os_mbuf *buf)
{
	if (adv_type != BLE_HCI_ADV_TYPE_ADV_NONCONN_IND) {
		// printf("non Mesh packet, dropped\n");
		++mystats.rx_nonmesh;
		return;
	}
	else {
		++mystats.rx_nonconn_ind;
	}

	// printf("got generic Mesh packet\n");

#if BT_MESH_EXTENDED_DEBUG
	BT_DBG("len %u: %s", buf->om_len, bt_hex(buf->om_data, buf->om_len));
#endif

	while (buf->om_len > 1) {
		++mystats.rx_cand;
		struct net_buf_simple_state state;
		u8_t len, type;

		len = net_buf_simple_pull_u8(buf);
		/* Check for early termination */
		if (len == 0) {
			++mystats.rx_cand_nolen;
			// printf("rx: len == 0\n");
			return;
		}

		if (len > buf->om_len) {
			++mystats.rx_cand_malformed;
			BT_WARN("AD malformed");
			return;
		}

		net_buf_simple_save(buf, &state);

		type = net_buf_simple_pull_u8(buf);

		// int mic;
		// memcpy(&mic, buf->om_data + (buf->om_len -4), 4);

		switch (type) {
		case BLE_HS_ADV_TYPE_MESH_MESSAGE:
			// printf("Mesh message +1\n");

			// mystats_inc_rx_adv_data(mic);
			++mystats.rx_mesh_adv_in;

			bt_mesh_net_recv(buf, rssi, BT_MESH_NET_IF_ADV);
			break;
#if MYNEWT_VAL(BLE_MESH_PB_ADV)
		case BLE_HS_ADV_TYPE_MESH_PROV:

			// printf("Mesh provisioning message +1\n");
			++mystats.rx_mesh_adv_prov;

			bt_mesh_pb_adv_recv(buf);
			break;
#endif
		case BLE_HS_ADV_TYPE_MESH_BEACON:

			++mystats.rx_mesh_adv_beacon;

			bt_mesh_beacon_recv(buf);
			break;
		default:
			++mystats.rx_type_nomesh;
			break;
		}

		net_buf_simple_restore(buf, &state);
		net_buf_simple_pull(buf, len);
	}
}

void bt_mesh_adv_init(void)
{
	int rc;

	/* Advertising should only be initialized once. Calling
	 * os_task init the second time will result in an assert. */
	if (adv_initialized) {
		return;
	}

	rc = os_mempool_init(&adv_buf_mempool, MYNEWT_VAL(BLE_MESH_ADV_BUF_COUNT),
			     BT_MESH_ADV_DATA_SIZE + BT_MESH_MBUF_HEADER_SIZE,
			     adv_buf_mem, "adv_buf_pool");
	assert(rc == 0);

	rc = os_mbuf_pool_init(&adv_os_mbuf_pool, &adv_buf_mempool,
			       BT_MESH_ADV_DATA_SIZE + BT_MESH_MBUF_HEADER_SIZE,
			       MYNEWT_VAL(BLE_MESH_ADV_BUF_COUNT));
	assert(rc == 0);

	ble_npl_eventq_init(&adv_queue);

#if MYNEWT
	os_task_init(&adv_task, "mesh_adv", mesh_adv_thread, NULL,
	             MYNEWT_VAL(BLE_MESH_ADV_TASK_PRIO), OS_WAIT_FOREVER,
	             g_blemesh_stack, ADV_STACK_SIZE);
#endif

	/* For BT5 controllers we can have fast advertising interval */
	if (ble_hs_hci_get_hci_version() >= BLE_HCI_VER_BCS_5_0) {
	    adv_int_min = ADV_INT_FAST_MS;
	}

	adv_initialized = true;
}

int
ble_adv_gap_mesh_cb(struct ble_gap_event *event, void *arg)
{
#if MYNEWT_VAL(BLE_EXT_ADV)
	struct ble_gap_ext_disc_desc *ext_desc;
#endif
	struct ble_gap_disc_desc *desc;
	struct os_mbuf *buf = NULL;

#if BT_MESH_EXTENDED_DEBUG
	BT_DBG("event->type %d", event->type);
#endif

	switch (event->type) {
#if MYNEWT_VAL(BLE_EXT_ADV)
	case BLE_GAP_EVENT_EXT_DISC:
		ext_desc = &event->ext_disc;
		buf = os_mbuf_get_pkthdr(&adv_os_mbuf_pool, 0);
		if (!buf || os_mbuf_append(buf, ext_desc->data, ext_desc->length_data)) {
			BT_ERR("Could not append data");
			goto done;
		}
		bt_mesh_scan_cb(&ext_desc->addr, ext_desc->rssi,
				ext_desc->legacy_event_type, buf);
		break;
#endif
	case BLE_GAP_EVENT_DISC:
		// printf("INCOMING GAP PACKET\n");
		desc = &event->disc;

		if (myfilter_drop(desc->addr.val)) {
			break;
		}

		++mystats.rx_all;
		buf = os_mbuf_get_pkthdr(&adv_os_mbuf_pool, 0);
		if (!buf || os_mbuf_append(buf, desc->data, desc->length_data)) {
			BT_ERR("Could not append data");
			goto done;
		}

		bt_mesh_scan_cb(&desc->addr, desc->rssi, desc->event_type, buf);
		break;
	default:
		break;
	}

done:
	if (buf) {
		os_mbuf_free_chain(buf);
	}

	return 0;
}

int bt_mesh_scan_enable(void)
{
#if MYNEWT_VAL(BLE_EXT_ADV)
	struct ble_gap_ext_disc_params uncoded_params =
		{ .itvl = MESH_SCAN_INTERVAL, .window = MESH_SCAN_WINDOW,
		.passive = 1 };

	BT_DBG("");

	return ble_gap_ext_disc(g_mesh_addr_type, 0, 0, 0, 0, 0,
				&uncoded_params, NULL, NULL, NULL);
#else
	struct ble_gap_disc_params scan_param =
		{ .passive = 1, .filter_duplicates = 0, .itvl =
		  MESH_SCAN_INTERVAL, .window = MESH_SCAN_WINDOW };

	BT_DBG("");

	return ble_gap_disc(g_mesh_addr_type, BLE_HS_FOREVER, &scan_param, NULL, NULL);
#endif
}

int bt_mesh_scan_disable(void)
{
	BT_DBG("");

	return ble_gap_disc_cancel();
}
