/**
 * Copyright (c) 2014 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/**
 * @brief BLE Heart Rate Collector application main file.
 *
 * This file contains the source code for a sample heart rate collector.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
// #include "nordic_common.h"
#include "nrf_sdm.h"
#include "ble.h"
#include "ble_hci.h"
// #include "ble_db_discovery.h"
// #include "ble_srv_common.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include <board-config.h>
#include "nrf_sdh_soc.h"
#include <ble_scan.h>
#include <ble_db_discovery.h>
#include <ble_conn_params.h>
// #include "nrf_pwr_mgmt.h"
// #include "app_util.h"
// #include "app_error.h"
// #include "peer_manager.h"
// #include "peer_manager_handler.h"
// #include "ble_hrs_c.h"
// #include "ble_bas_c.h"
// #include "app_util.h"
// #include "app_timer.h"
// #include "bsp_btn_ble.h"
// #include "fds.h"
// #include "nrf_fstorage.h"
// #include "ble_conn_state.h"
// #include "nrf_ble_gatt.h"
// #include "nrf_ble_lesc.h"
// #include "nrf_log.h"
// #include "nrf_log_ctrl.h"
// #include "nrf_log_default_backends.h"
// #include "nrf_ble_scan.h"

#include <ble_scan.h>
#include <ble_gq.h>

#include <bluetooth/services/ble_bas.h>
#include <bluetooth/services/ble_dis.h>
#include <ble_hrs_c.h>

#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/bluetooth/uuid.h>

#include <bm_buttons.h>

#include <board-config.h>

#include <hal/nrf_gpio.h>

LOG_MODULE_REGISTER(app, 3);

// <e> NRF_BLE_SCAN_ENABLED - nrf_ble_scan - Scanning Module
//==========================================================
#ifndef NRF_BLE_SCAN_ENABLED
#define NRF_BLE_SCAN_ENABLED 1
#endif
// <o> NRF_BLE_SCAN_BUFFER - Data length for an advertising set.
#ifndef NRF_BLE_SCAN_BUFFER
#define NRF_BLE_SCAN_BUFFER 31
#endif

// <o> NRF_BLE_SCAN_NAME_MAX_LEN - Maximum size for the name to search in the advertisement report.
#ifndef NRF_BLE_SCAN_NAME_MAX_LEN
#define NRF_BLE_SCAN_NAME_MAX_LEN 32
#endif

// <o> NRF_BLE_SCAN_SHORT_NAME_MAX_LEN - Maximum size of the short name to search for in the
// advertisement report.
#ifndef NRF_BLE_SCAN_SHORT_NAME_MAX_LEN
#define NRF_BLE_SCAN_SHORT_NAME_MAX_LEN 32
#endif

// <o> NRF_BLE_SCAN_SCAN_INTERVAL - Scanning interval. Determines the scan interval in units of
// 0.625 millisecond.
#ifndef NRF_BLE_SCAN_SCAN_INTERVAL
#define NRF_BLE_SCAN_SCAN_INTERVAL 160
#endif

// <o> NRF_BLE_SCAN_SCAN_DURATION - Duration of a scanning session in units of 10 ms. Range: 0x0001
// - 0xFFFF (10 ms to 10.9225 ms). If set to 0x0000, the scanning continues until it is explicitly
// disabled.
#ifndef NRF_BLE_SCAN_SCAN_DURATION
#define NRF_BLE_SCAN_SCAN_DURATION 0
#endif

// <o> NRF_BLE_SCAN_SCAN_WINDOW - Scanning window. Determines the scanning window in units of 0.625
// millisecond.
#ifndef NRF_BLE_SCAN_SCAN_WINDOW
#define NRF_BLE_SCAN_SCAN_WINDOW 80
#endif

// <o> NRF_BLE_SCAN_MIN_CONNECTION_INTERVAL - Determines minimum connection interval in
// milliseconds.
#ifndef NRF_BLE_SCAN_MIN_CONNECTION_INTERVAL
#define NRF_BLE_SCAN_MIN_CONNECTION_INTERVAL 100
#endif

// <o> NRF_BLE_SCAN_MAX_CONNECTION_INTERVAL - Determines maximum connection interval in
// milliseconds.
#ifndef NRF_BLE_SCAN_MAX_CONNECTION_INTERVAL
#define NRF_BLE_SCAN_MAX_CONNECTION_INTERVAL 500
#endif

// <o> NRF_BLE_SCAN_SLAVE_LATENCY - Determines the slave latency in counts of connection events.
#ifndef NRF_BLE_SCAN_SLAVE_LATENCY
#define NRF_BLE_SCAN_SLAVE_LATENCY 5
#endif

// <o> NRF_BLE_SCAN_SUPERVISION_TIMEOUT - Determines the supervision time-out in units of 10
// millisecond.
#ifndef NRF_BLE_SCAN_SUPERVISION_TIMEOUT
#define NRF_BLE_SCAN_SUPERVISION_TIMEOUT 6200
#endif

#define APP_BLE_CONN_CFG_TAG 1 /**< A tag identifying the SoftDevice BLE configuration. */

#define APP_BLE_OBSERVER_PRIO                                                                      \
	3 /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_SOC_OBSERVER_PRIO                                                                      \
	1 /**< Applications' SoC observer priority. You shouldn't need to modify this value. */

#define LESC_DEBUG_MODE                                                                            \
	0 /**< Set to 1 to use LESC debug keys, allows you to use a sniffer to inspect traffic. */

#define SEC_PARAM_BOND		   1 /**< Perform bonding. */
#define SEC_PARAM_MITM		   0 /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC		   1 /**< LE Secure Connections enabled. */
#define SEC_PARAM_KEYPRESS	   0 /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES  BLE_GAP_IO_CAPS_NONE /**< No I/O capabilities. */
#define SEC_PARAM_OOB		   0			/**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE	   7  /**< Minimum encryption key size in octets. */
#define SEC_PARAM_MAX_KEY_SIZE	   16 /**< Maximum encryption key size in octets. */
#define NRF_BLE_SCAN_FILTER_ENABLE 1

#define SCAN_DURATION_WHITELIST	    3000 /**< Duration of the scanning in units of 10 milliseconds. */
#define BLE_UUID_HEART_RATE_SERVICE 0x180D
#define TARGET_UUID                                                                                \
	BLE_UUID_HEART_RATE_SERVICE /**< Target device uuid that application is looking for. */

/**@brief Macro to unpack 16bit unsigned UUID from octet stream. */
#define UUID16_EXTRACT(DST, SRC)                                                                   \
	do {                                                                                       \
		(*(DST)) = (SRC)[1];                                                               \
		(*(DST)) <<= 8;                                                                    \
		(*(DST)) |= (SRC)[0];                                                              \
	} while (0)

// NRF_BLE_GQ_DEF(m_ble_gatt_queue, /**< BLE GATT Queue instance. */
//	       NRF_SDH_BLE_CENTRAL_LINK_COUNT, NRF_BLE_GQ_QUEUE_SIZE);
BLE_HRS_C_DEF(m_hrs_c); /**< Structure used to identify the heart rate client module. */
// BLE_BAS_C_DEF(m_bas_c); /**< Structure used to identify the Battery Service client module. */
BLE_GQ_DEF(m_ble_gatt_queue);
BLE_DB_DISCOVERY_DEF(m_db_disc); /**< DB discovery module instance. */
NRF_BLE_SCAN_DEF(m_scan);	 /**< Scanning module instance. */

static uint16_t m_conn_handle;		 /**< Current connection handle. */
static bool m_whitelist_disabled;	 /**< True if whitelist has been temporarily disabled. */
static bool m_memory_access_in_progress; /**< Flag to keep track of ongoing operations on persistent
					    memory. */

/**< Scan parameters requested for scanning and connection. */
static ble_gap_scan_params_t const m_scan_param = {
	.active = 0x01,
	.interval = BLE_GAP_SCAN_INTERVAL_US_MIN * 6,
	.window = BLE_GAP_SCAN_WINDOW_US_MIN * 6,

	.filter_policy = BLE_GAP_SCAN_FP_ACCEPT_ALL,
	.timeout = BLE_GAP_SCAN_TIMEOUT_UNLIMITED,
	.scan_phys = BLE_GAP_PHY_AUTO,
};

/**@brief Names which the central applications will scan for, and which will be advertised by the
 * peripherals. if these are set to empty strings, the UUIDs defined below will be used
 */
static char const m_target_periph_name[] =
	"SP_nRF_BM_HRS"; /**< If you want to connect to a peripheral using a
	       given advertising name, type its name here. */
static bool is_connect_per_addr =
	false; /**< If you want to connect to a peripheral with a given address, set this to true
		  and put the correct address in the variable below. */

static ble_gap_addr_t const m_target_periph_addr = {
	/* Possible values for addr_type:
	   BLE_GAP_ADDR_TYPE_PUBLIC,
	   BLE_GAP_ADDR_TYPE_RANDOM_STATIC,
	   BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE,
	   BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE. */
	.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC,
	.addr = {0x78, 0xE7, 0xF8, 0x06, 0xC5, 0xD8}};

static void scan_start(void);

/**@brief Function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num     Line number of the failing ASSERT call.
 * @param[in] p_file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name)
{
	// app_error_handler(0xDEADBEEF, line_num, p_file_name);
	LOG_ERR("assert_nrf_callback %s:%" PRIu16, p_file_name, line_num);
}

/**@brief Function for handling the Heart Rate Service Client and Battery Service Client errors.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void service_error_handler(uint32_t nrf_error)
{
	// APP_ERROR_HANDLER(nrf_error);
	LOG_ERR("service_error_handler error %" PRIu32, nrf_error);
}

/**@brief Function for handling database discovery events.
 *
 * @details This function is callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function should forward the events
 *          to their respective services.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */

static void db_disc_handler(struct ble_db_discovery_evt *p_evt)
{
	ble_hrs_on_db_disc_evt(&m_hrs_c, p_evt);
	// ble_bas_on_db_disc_evt(&m_bas_c, p_evt);
}

/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
/*
static void pm_evt_handler(pm_evt_t const *p_evt)
{
	pm_handler_on_pm_evt(p_evt);
	pm_handler_disconnect_on_sec_failure(p_evt);
	pm_handler_flash_clean(p_evt);

	switch (p_evt->evt_id) {
	case PM_EVT_PEERS_DELETE_SUCCEEDED:
		// Bonds are deleted. Start scanning.
		scan_start();
		break;

	default:
		break;
	}
}
*/

/**
 * @brief Function for shutdown events.
 *
 * @param[in]   event       Shutdown type.
 */
/*
static bool shutdown_handler(nrf_pwr_mgmt_evt_t event)
{
	int err;

	err = bsp_indication_set(BSP_INDICATE_IDLE);
	// APP_ERROR_CHECK(err);

	switch (event) {
	case NRF_PWR_MGMT_EVT_PREPARE_WAKEUP:
		// Prepare wakeup buttons.
		err = bsp_btn_ble_sleep_mode_prepare();
		// APP_ERROR_CHECK(err);
		break;
	default:
		break;
	}

	return true;
}
	*/

// NRF_PWR_MGMT_HANDLER_REGISTER(shutdown_handler, APP_SHUTDOWN_HANDLER_PRIORITY);

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context)
{
	int err;
	ble_gap_evt_t const *p_gap_evt = &p_ble_evt->evt.gap_evt;

	switch (p_ble_evt->header.evt_id) {
	case BLE_GAP_EVT_CONNECTED: {
		LOG_INF("Connected.");

		m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

		// Discover peer's services.
		err = ble_db_discovery_start(
			&m_db_disc,
			p_ble_evt->evt.gap_evt.conn_handle); // APP_ERROR_CHECK(err);

		if (err != 0) {
			LOG_ERR("db discovery start failed %d", err);
		}

		// err = bsp_indication_set(BSP_INDICATE_CONNECTED);
		//  APP_ERROR_CHECK(err);

		// if (ble_conn_state_central_conn_count() < CONFIG_NRF_SDH_BLE_CENTRAL_LINK_COUNT)
		// { 	scan_start();
		// }
	} break;

	case BLE_GAP_EVT_DISCONNECTED: {
		LOG_INF("Disconnected, reason 0x%x.", p_gap_evt->params.disconnected.reason);

		// err = bsp_indication_set(BSP_INDICATE_IDLE);
		//  APP_ERROR_CHECK(err);

		// if (ble_conn_state_central_conn_count() < 1) {
		scan_start();
		//}
	} break;

	case BLE_GAP_EVT_TIMEOUT: {
		if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN) {
			LOG_INF("Connection Request timed out.");
		}
	} break;

	case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
		// Accepting parameters requested by peer.
		LOG_INF("ble gap event connection parameter update request");
		err = sd_ble_gap_conn_param_update(
			p_gap_evt->conn_handle,
			&p_gap_evt->params.conn_param_update_request.conn_params);
		// APP_ERROR_CHECK(err);
		break;

	case BLE_GAP_EVT_PHY_UPDATE_REQUEST: {
		LOG_INF("PHY update request.");
		ble_gap_phys_t const phys = {
			.rx_phys = BLE_GAP_PHY_AUTO,
			.tx_phys = BLE_GAP_PHY_AUTO,
		};
		err = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
		// APP_ERROR_CHECK(err);
	} break;

	case BLE_GATTC_EVT_TIMEOUT:
		// Disconnect on GATT Client timeout event.
		LOG_INF("GATT Client Timeout.");
		err = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
					    BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
		// APP_ERROR_CHECK(err);
		break;

	case BLE_GATTS_EVT_TIMEOUT:
		// Disconnect on GATT Server timeout event.
		LOG_INF("GATT Server Timeout.");
		err = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
					    BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
		// APP_ERROR_CHECK(err);
		break;

	case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
		LOG_INF("BLE_GAP_EVT_SEC_PARAMS_REQUEST");
		break;

	case BLE_GAP_EVT_AUTH_KEY_REQUEST:
		LOG_INF("BLE_GAP_EVT_AUTH_KEY_REQUEST");
		break;

	case BLE_GAP_EVT_LESC_DHKEY_REQUEST:
		LOG_INF("BLE_GAP_EVT_LESC_DHKEY_REQUEST");
		break;

	case BLE_GAP_EVT_AUTH_STATUS:
		LOG_INF("BLE_GAP_EVT_AUTH_STATUS: status=0x%x bond=0x%x lv4: %d "
			"kdist_own:0x%x kdist_peer:0x%x",
			p_ble_evt->evt.gap_evt.params.auth_status.auth_status,
			p_ble_evt->evt.gap_evt.params.auth_status.bonded,
			p_ble_evt->evt.gap_evt.params.auth_status.sm1_levels.lv4,
			*((uint8_t *)&p_ble_evt->evt.gap_evt.params.auth_status.kdist_own),
			*((uint8_t *)&p_ble_evt->evt.gap_evt.params.auth_status.kdist_peer));
		break;
	case BLE_GAP_EVT_ADV_REPORT:
		ble_gap_evt_adv_report_t *adv_report = &p_gap_evt->params.adv_report;
		// LOG_INF("Peer address: 0x%02X%02X%02X%02X%02X%02X",
		// adv_report->peer_addr.addr[0], 	adv_report->peer_addr.addr[1],
		// adv_report->peer_addr.addr[2], 	adv_report->peer_addr.addr[3],
		// adv_report->peer_addr.addr[4], 	adv_report->peer_addr.addr[5]);
		//   LOG_INF("Adv data: %d", adv_report->data);
		//  LOG_INF("RSSI: %d", adv_report->rssi);
		if (!memcmp(adv_report->peer_addr.addr, m_target_periph_addr.addr,
			    sizeof(adv_report->peer_addr.addr))) {
			// nrf_ble_scan_stop();
			LOG_INF("Connecting to Peripheral");
			err = sd_ble_gap_connect(&adv_report->peer_addr, &m_scan.scan_params,
						 &m_scan.conn_params, CONFIG_NRF_SDH_BLE_CONN_TAG);
			if (err != 0) {
				LOG_ERR("Failed to connect %d", err);
			}
		}
		break;

	default:
		LOG_WRN("Unhandled ble event %d", p_ble_evt->header.evt_id);
		break;
	}
}

/**@brief SoftDevice SoC event handler.
 *
 * @param[in]   evt_id      SoC event.
 * @param[in]   p_context   Context.
 */
static void soc_evt_handler(uint32_t evt_id, void *p_context)
{
	switch (evt_id) {
	case NRF_EVT_FLASH_OPERATION_SUCCESS:
		/* fall through */
	case NRF_EVT_FLASH_OPERATION_ERROR:

		if (m_memory_access_in_progress) {
			m_memory_access_in_progress = false;
			scan_start();
		}
		break;

	default:
		// No implementation needed.
		break;
	}
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
	int err;

	err = nrf_sdh_enable_request();
	// APP_ERROR_CHECK(err);

	// Configure the BLE stack using the default settings.
	// Fetch the start address of the application RAM.
	// uint32_t ram_start = 0;
	// err = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
	// APP_ERROR_CHECK(err);

	// Enable BLE stack.
	err = nrf_sdh_ble_enable(CONFIG_NRF_SDH_BLE_CONN_TAG);
	if (err != 0) {
		LOG_ERR("nrf_sdh_ble_enable failed %d", err);
	}
	// APP_ERROR_CHECK(err);

	// Register handlers for BLE and SoC events.
	NRF_SDH_BLE_OBSERVER(m_ble_observer, ble_evt_handler, NULL, APP_BLE_OBSERVER_PRIO);
	// NRF_SDH_SOC_OBSERVER(m_soc_observer, soc_evt_handler, NULL, APP_SOC_OBSERVER_PRIO);
}

/**@brief Function for the Peer Manager initialization.
 */
/*
static void peer_manager_init(void)
{
	ble_gap_sec_params_t sec_param;
	int err;

	err = pm_init();
	// APP_ERROR_CHECK(err);

	memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

	// Security parameters to be used for all security procedures.
	sec_param.bond = SEC_PARAM_BOND;
	sec_param.mitm = SEC_PARAM_MITM;
	sec_param.lesc = SEC_PARAM_LESC;
	sec_param.keypress = SEC_PARAM_KEYPRESS;
	sec_param.io_caps = SEC_PARAM_IO_CAPABILITIES;
	sec_param.oob = SEC_PARAM_OOB;
	sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
	sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
	sec_param.kdist_own.enc = 1;
	sec_param.kdist_own.id = 1;
	sec_param.kdist_peer.enc = 1;
	sec_param.kdist_peer.id = 1;

	err = pm_sec_params_set(&sec_param);
	// APP_ERROR_CHECK(err);

	err = pm_register(pm_evt_handler);
	// APP_ERROR_CHECK(err);
}
*/

/** @brief Clear bonding information from persistent storage
 */
static void delete_bonds(void)
{
	int err;

	LOG_INF("Erase bonds!");

	// err = pm_peers_delete();
	// APP_ERROR_CHECK(err);
}

/**@brief Function for disabling the use of whitelist for scanning.
 */
static void whitelist_disable(void)
{
	if (!m_whitelist_disabled) {
		LOG_INF("Whitelist temporarily disabled.");
		m_whitelist_disabled = true;
		nrf_ble_scan_stop();
		scan_start();
	}
}

static void button_handler_whitelist_off(uint8_t pin, uint8_t action)
{
	LOG_INF("Button whitelist off");
	whitelist_disable();
}

static void button_handler_disconnect(uint8_t pin, uint8_t action)
{
	LOG_INF("Button disconnect");

	int err = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
	if (err != 0) {
		LOG_ERR("ble gap disconnect failed %d", err);
	}
}

static void button_handler_sleep(uint8_t pin, uint8_t action)
{
	LOG_INF("Button sleep");
	k_sleep(K_SECONDS(10));

	// nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
}

/**@brief Heart Rate Collector Handler.
 */
static void hrs_c_evt_handler(struct ble_hrs_c *hrs, struct ble_hrs_c_evt *evt)
{

	int err;

	switch (evt->evt_type) {
	case BLE_HRS_C_EVT_DISCOVERY_COMPLETE: {
		LOG_INF("Heart rate service discovered.");

		err = ble_hrs_c_handles_assign(hrs, evt->conn_handle, &evt->params.peer_db);
		if (err != 0) {
			LOG_ERR("ble_hrs_c_handles_assign error: %d", err);
		}

		// Heart rate service discovered. Enable notification of Heart Rate Measurement.
		err = ble_hrs_c_hrm_notif_enable(hrs);
		if (err != 0) {
			LOG_ERR("ble_hrs_c_hrm_notif_enable error: %d", err);
		}
	} break;

	case BLE_HRS_C_EVT_HRM_NOTIFICATION: {
		LOG_INF("Heart Rate = %d.", evt->params.hrm.hr_value);
		uint32_t rr_avg = 0;
		if (evt->params.hrm.rr_intervals_cnt != 0) {
			uint32_t rr_avg = 0;
			for (uint32_t i = 0; i < evt->params.hrm.rr_intervals_cnt; i++) {
				rr_avg += evt->params.hrm.rr_intervals[i];
			}
			rr_avg = rr_avg / evt->params.hrm.rr_intervals_cnt;
			LOG_INF("rr_interval (avg) = %d.", rr_avg);
		}
	} break;

	default:
		LOG_WRN("Unhandled hrs event %d", evt->evt_type);
		break;
	}
}

/**@brief Battery level Collector Handler.
 */
/*
static void bas_c_evt_handler(ble_bas_c_t *p_bas_c, ble_bas_c_evt_t *p_bas_c_evt)
{
	int err;

	switch (p_bas_c_evt->evt_type) {
	case BLE_BAS_C_EVT_DISCOVERY_COMPLETE: {
		err = ble_bas_c_handles_assign(p_bas_c, p_bas_c_evt->conn_handle,
					       &p_bas_c_evt->params.bas_db);
		if (err != 0) {
			LOG_ERR("ble_bas_c_handles_assign error: %d", err);
		}

		// Battery service discovered. Enable notification of Battery Level.
		LOG_INF("Battery Service discovered. Reading battery level.");

		err = ble_bas_c_bl_read(p_bas_c);
		if (err != 0) {
			LOG_ERR("ble_bas_c_bl_read error: %d", err);
		}

		// LOG_INF("Enabling Battery Level Notification.");
		err = ble_bas_c_bl_notif_enable(p_bas_c);
		if (err != 0) {
			LOG_ERR("ble_bas_c_bl_notif_enable error: %d", err);
		}

	} break;

	case BLE_BAS_C_EVT_BATT_NOTIFICATION:
		LOG_INF("Battery Level received %d %%.", p_bas_c_evt->params.battery_level);
		break;

	case BLE_BAS_C_EVT_BATT_READ_RESP:
		LOG_INF("Battery Level Read as %d %%.", p_bas_c_evt->params.battery_level);
		break;

	default:
		break;
	}
}
*/

/**
 * @brief Heart rate collector initialization.
 */
static void hrs_c_init(void)
{

	ble_hrs_c_init(&m_hrs_c, &(struct ble_hrs_c_config){.error_handler = service_error_handler,
							    .evt_handler = hrs_c_evt_handler,
							    .p_gatt_queue = &m_ble_gatt_queue});
	// APP_ERROR_CHECK(err);
}

/**
 * @brief Battery level collector initialization.
 */
static void bas_c_init(void)
{
	// ble_bas_c_init_t bas_c_init_obj;
	//
	// bas_c_init_obj.evt_handler = bas_c_evt_handler;
	// bas_c_init_obj.error_handler = service_error_handler;
	// bas_c_init_obj.p_gatt_queue = &m_ble_gatt_queue;
	//
	// int err = ble_bas_c_init(&m_bas_c, &bas_c_init_obj);
	// APP_ERROR_CHECK(err);
}

/**
 * @brief Database discovery collector initialization.
 */
static void db_discovery_init(void)
{
	struct ble_db_discovery_init db_init = {0};

	db_init.evt_handler = db_disc_handler;
	db_init.p_gatt_queue = &m_ble_gatt_queue;

	int err = ble_db_discovery_init(&db_init);
	if (err != 0) {
		LOG_ERR("db discovery init failed %d", err);
	}

	// APP_ERROR_CHECK(err);
}

/**@brief Retrieve a list of peer manager peer IDs.
 *
 * @param[inout] p_peers   The buffer where to store the list of peer IDs.
 * @param[inout] p_size    In: The size of the @p p_peers buffer.
 *                         Out: The number of peers copied in the buffer.
 */

/*
static void peer_list_get(pm_peer_id_t *p_peers, uint32_t *p_size)
{
       pm_peer_id_t peer_id;
       uint32_t peers_to_copy;

       peers_to_copy = (*p_size < BLE_GAP_WHITELIST_ADDR_MAX_COUNT)
			       ? *p_size
			       : BLE_GAP_WHITELIST_ADDR_MAX_COUNT;

       peer_id = pm_next_peer_id_get(PM_PEER_ID_INVALID);
       *p_size = 0;

       while ((peer_id != PM_PEER_ID_INVALID) && (peers_to_copy--)) {
	       p_peers[(*p_size)++] = peer_id;
	       peer_id = pm_next_peer_id_get(peer_id);
       }
}
*/
/*
static void whitelist_load()
{
	int ret;
	pm_peer_id_t peers[8];
	uint32_t peer_cnt;

	memset(peers, PM_PEER_ID_INVALID, sizeof(peers));
	peer_cnt = (sizeof(peers) / sizeof(pm_peer_id_t));

	// Load all peers from flash and whitelist them.
	peer_list_get(peers, &peer_cnt);

	ret = pm_whitelist_set(peers, peer_cnt);
	// APP_ERROR_CHECK(ret);

	// Setup the device identies list.
	// Some SoftDevices do not support this feature.
	ret = pm_device_identities_list_set(peers, peer_cnt);
	if (ret != NRF_ERROR_NOT_SUPPORTED) {
		// APP_ERROR_CHECK(ret);
	}
}
*/

/*
static void on_whitelist_req(void)
{
	int err;

	// Whitelist buffers.
	ble_gap_addr_t whitelist_addrs[8] = {0};
	ble_gap_irk_t whitelist_irks[8] = {0};

	uint32_t addr_cnt = (sizeof(whitelist_addrs) / sizeof(ble_gap_addr_t));
	uint32_t irk_cnt = (sizeof(whitelist_irks) / sizeof(ble_gap_irk_t));

	// Reload the whitelist and whitelist all peers.
	whitelist_load();

	// Get the whitelist previously set using pm_whitelist_set().
	err = pm_whitelist_get(whitelist_addrs, &addr_cnt, whitelist_irks, &irk_cnt);

	if (((addr_cnt == 0) && (irk_cnt == 0)) || (m_whitelist_disabled)) {
		// Don't use whitelist.
		err = nrf_ble_scan_params_set(&m_scan, NULL);
		// APP_ERROR_CHECK(err);
	}
}
	*/

/**@brief Function to start scanning.
 */
static void scan_start(void)
{
	int err;

	// if (nrf_fstorage_is_busy(NULL)) {
	//	m_memory_access_in_progress = true;
	//	return;
	// }

	LOG_INF("Starting scan.");

	err = nrf_ble_scan_start(&m_scan);
	if (err != 0) {
		LOG_ERR("nrf_ble_scan_start failed %d", err);
	}
	// APP_ERROR_CHECK(err);

	// err = bsp_indication_set(BSP_INDICATE_SCANNING);
	//  APP_ERROR_CHECK(err);
}

/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the
 * application up.
 */
static void buttons_leds_init(bool *p_erase_bonds)
{
	int err;

	static struct bm_buttons_config configs[] = {
		{
			.pin_number = BOARD_PIN_BTN_0,
			.active_state = BM_BUTTONS_ACTIVE_LOW,
			.pull_config = BM_BUTTONS_PIN_PULLUP,
			.handler = button_handler_whitelist_off,
		},
		{
			.pin_number = BOARD_PIN_BTN_1,
			.active_state = BM_BUTTONS_ACTIVE_LOW,
			.pull_config = BM_BUTTONS_PIN_PULLUP,
			.handler = button_handler_disconnect,
		},
		{
			.pin_number = BOARD_PIN_BTN_2,
			.active_state = BM_BUTTONS_ACTIVE_LOW,
			.pull_config = BM_BUTTONS_PIN_PULLUP,
			.handler = button_handler_sleep,
		},
	};

	err = bm_buttons_init(configs, ARRAY_SIZE(configs), BM_BUTTONS_DETECTION_DELAY_MIN_US);
	if (err) {
		LOG_ERR("Failed to initialize buttons, err: %d", err);
	}

	err = bm_buttons_enable();
	if (err) {
		LOG_ERR("Failed to enable buttons, err: %d", err);
	}

	nrf_gpio_cfg_output(BOARD_PIN_LED_0);

	//*p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);

	*p_erase_bonds = false;
}

/**@brief Function for initializing the nrf log module.
 */

/**@brief Function for initializing the power management module. */
static void power_management_init(void)
{
	int err;
	// err = nrf_pwr_mgmt_init();
	//  APP_ERROR_CHECK(err);
}

/**@brief GATT module event handler.
 */

static void conn_params_evt_handler(const struct ble_conn_params_evt *evt)
{
	switch (evt->id) {
	case BLE_CONN_PARAMS_EVT_UPDATED: {
		LOG_INF("GATT ATT MTU on connection 0x%x changed to %d.", evt->conn_handle,
			evt->att_mtu);
	} break;

	case BLE_CONN_PARAMS_EVT_DATA_LENGTH_UPDATED: {
		LOG_INF("Data length for connection 0x%x updated to %d.", evt->conn_handle,
			evt->data_length.rx);
	} break;

	default:
		LOG_WRN("unhandled conn params event %d", evt->id);
		break;
	}
}

static void scan_evt_handler(struct scan_evt const *p_scan_evt)
{
	int err;
	switch (p_scan_evt->scan_evt_id) {
	case NRF_BLE_SCAN_EVT_WHITELIST_REQUEST: {
		// on_whitelist_req();
		m_whitelist_disabled = false;
		LOG_INF("Whitelist request.");
	} break;

	case NRF_BLE_SCAN_EVT_CONNECTING_ERROR: {
		err = p_scan_evt->params.connecting_err.err_code;
		LOG_INF("Scan connecting error");
		// APP_ERROR_CHECK(err);
	} break;

	case NRF_BLE_SCAN_EVT_SCAN_TIMEOUT: {
		LOG_INF("Scan timed out.");
		scan_start();
	} break;

	case NRF_BLE_SCAN_EVT_FILTER_MATCH:
		LOG_INF("Scan filter match");
		break;
	case NRF_BLE_SCAN_EVT_WHITELIST_ADV_REPORT:
		LOG_INF("Whitelist advertise report.");
		break;
	case NRF_BLE_SCAN_EVT_CONNECTED: {
		ble_gap_evt_connected_t const *p_connected =
			p_scan_evt->params.connected.p_connected;
		// Scan is automatically stopped by the connection.
		LOG_INF("Connecting to target %02x%02x%02x%02x%02x%02x",
			p_connected->peer_addr.addr[0], p_connected->peer_addr.addr[1],
			p_connected->peer_addr.addr[2], p_connected->peer_addr.addr[3],
			p_connected->peer_addr.addr[4], p_connected->peer_addr.addr[5]);
	} break;

	default:
		LOG_WRN("Unhandled scan event %d", p_scan_evt->scan_evt_id);
		break;
	}
}

/**@brief Function for initializing the timer.
 */
static void timer_init(void)
{
	// int err = app_timer_init();
	// APP_ERROR_CHECK(err);
}

/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void)
{
	int err = ble_conn_params_evt_handler_set(conn_params_evt_handler);
	if (err != 0) {
		LOG_ERR("ble_conn_params_evt_handler_set failed %d", err);
	}
}

/**@brief Function for initialization scanning and setting filters.
 */
static void scan_init(void)
{
	int err;
	struct nrf_ble_scan_init init_scan = {0};

	init_scan.p_scan_param = &m_scan_param;
	init_scan.connect_if_match = true;
	init_scan.conn_cfg_tag = CONFIG_NRF_SDH_BLE_CONN_TAG;

	err = nrf_ble_scan_params_set(&m_scan, NULL);
	if (err != 0) {
		LOG_ERR("nrf_ble_scan_params_set failed %d", err);
	}

	err = nrf_ble_scan_init(&m_scan, &init_scan, scan_evt_handler);
	if (err != 0) {
		LOG_ERR("nrf_ble_scan_init failed %d", err);
	}
	// APP_ERROR_CHECK(err);

	ble_uuid_t uuid = {
		.uuid = TARGET_UUID,
		.type = BLE_UUID_TYPE_BLE,
	};

	// err = nrf_ble_scan_filter_set(&m_scan, SCAN_UUID_FILTER, &uuid);
	// if (err != 0) {
	//	LOG_ERR("nrf_ble_scan_filter_set uuid failed %d", err);
	// }
	//  APP_ERROR_CHECK(err);

	if (strlen(m_target_periph_name) != 0) {
		err = nrf_ble_scan_filter_set(&m_scan, SCAN_NAME_FILTER, m_target_periph_name);
		if (err != 0) {
			LOG_ERR("nrf_ble_scan_filter_set name failed %d", err);
		}
		// APP_ERROR_CHECK(err);
	}

	if (is_connect_per_addr) {
		err = nrf_ble_scan_filter_set(&m_scan, SCAN_ADDR_FILTER, m_target_periph_addr.addr);
		if (err != 0) {
			LOG_ERR("nrf_ble_scan_filter_set address failed %d", err);
		}
		// APP_ERROR_CHECK(err);
	}

	err = nrf_ble_scan_filters_enable(&m_scan, NRF_BLE_SCAN_ALL_FILTER, false);
	//  APP_ERROR_CHECK(err);
}

/**@brief Function for handling the idle state (main loop).
 *
 * @details Handle any pending log operation(s), then sleep until the next event occurs.
 */
static void idle_state_handle(void)
{
	int err;

	// err = nrf_ble_lesc_request_handler();
	//  APP_ERROR_CHECK(err);

	// NRF_LOG_FLUSH();
	// nrf_pwr_mgmt_run();
}

/**@brief Function for starting a scan, or instead trigger it from peer manager (after
 *        deleting bonds).
 *
 * @param[in] p_erase_bonds Pointer to a bool to determine if bonds will be deleted before scanning.
 */
void scanning_start(bool *p_erase_bonds)
{
	// Start scanning for peripherals and initiate connection
	// with devices that advertise GATT Service UUID.
	if (*p_erase_bonds == true) {
		// Scan is started by the PM_EVT_PEERS_DELETE_SUCCEEDED event.
		delete_bonds();
	} else {
		scan_start();
	}
}

int main(void)
{
	bool erase_bonds;

	log_panic();

	LOG_INF("main");

	// Initialize.
	timer_init();
	power_management_init();
	buttons_leds_init(&erase_bonds);
	ble_stack_init();
	gatt_init();
	// peer_manager_init();
	db_discovery_init();
	hrs_c_init();
	bas_c_init();
	scan_init();

	// Start execution.
	LOG_INF("Heart Rate collector example started.");
	scanning_start(&erase_bonds);

	// Enter main loop.
	for (;;) {
		idle_state_handle();
	}
}
