/*
 * Copyright (c) 2012 - 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
/**
 * @file
 *
 * @defgroup ble_hrs_c Heart Rate Service Client
 * @{
 * @ingroup  ble_sdk_srv
 * @brief    Heart Rate Service Client module.
 *
 * @details  This module contains the APIs and types exposed by the Heart Rate Service Client
 *           module. The application can use these APIs and types to perform the discovery of
 *           Heart Rate Service at the peer and to interact with it.
 *
 * @warning  Currently, this module only supports the Heart Rate Measurement characteristic. This
 *           means that it is able to enable notification of the characteristic at the peer and
 *           is able to receive Heart Rate Measurement notifications from the peer. It does not
 *           support the Body Sensor Location and the Heart Rate Control Point characteristics.
 *           When a Heart Rate Measurement is received, this module decodes only the
 *           Heart Rate Measurement value field (both 8-bit and 16-bit) and provides it to
 *           the application.
 *
 * @note    The application must register this module as the BLE event observer by using the
 *          NRF_SDH_BLE_OBSERVER macro. Example:
 *          @code
 *              ble_hrs_c_t instance;
 *              NRF_SDH_BLE_OBSERVER(anything, BLE_HRS_C_BLE_OBSERVER_PRIO,
 *                                   ble_hrs_c_on_ble_evt, &instance);
 *          @endcode
 */

#ifndef BLE_HRS_C_H__
#define BLE_HRS_C_H__

#include <stdint.h>
#include "ble.h"
#include "ble_db_discovery.h"
#include "ble_srv_common.h"
#include <ble_gq.h>
#include "nrf_sdh_ble.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BLE_HRS_C_BLE_OBSERVER_PRIO
#define BLE_HRS_C_BLE_OBSERVER_PRIO 2
#endif

/**
 * @brief   Macro for defining a ble_hrs_c instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_HRS_C_DEF(_name)                                                                       \
	static struct ble_hrs_c _name;                                                             \
	NRF_SDH_BLE_OBSERVER(_name##_obs, ble_hrs_c_on_ble_evt, &_name, BLE_HRS_C_BLE_OBSERVER_PRIO)

/**
 * @brief Macro for defining multiple ble_hrs_c instances.
 *
 * @param   _name   Name of the array of instances.
 * @param   _cnt    Number of instances to define.
 * @hideinitializer
 */
#define BLE_HRS_C_ARRAY_DEF(_name, _cnt)                                                           \
	static ble_hrs_c_t _name[_cnt];                                                            \
	NRF_SDH_BLE_OBSERVERS(_name##_obs, BLE_HRS_C_BLE_OBSERVER_PRIO, ble_hrs_c_on_ble_evt,      \
			      &_name, _cnt)

/**
 * @brief  Maximum number of RR intervals to be decoded for each HRM notifications (any extra RR
 * intervals are ignored).
 *
 * This define should be defined in the sdk_config.h file to override the default.
 */
#ifndef BLE_HRS_C_RR_INTERVALS_MAX_CNT
#define BLE_HRS_C_RR_INTERVALS_MAX_CNT 20
#endif

/**
 * @defgroup hrs_c_enums Enumerations
 * @{
 */

/**
 * @brief HRS Client event type.
 */
typedef enum {
	BLE_HRS_C_EVT_DISCOVERY_COMPLETE =
		1, /** Event indicating that the Heart Rate Service was discovered at the peer. */
	BLE_HRS_C_EVT_HRM_NOTIFICATION /** Event indicating that a notification of the Heart Rate
					  Measurement characteristic was received from the peer. */
} ble_hrs_c_evt_type_t;

/** @} */

/**
 * @defgroup hrs_c_structs Structures
 * @{
 */

/**
 * @brief Structure containing the Heart Rate Measurement received from the peer.
 */
typedef struct {
	/** Heart Rate Value. */
	uint16_t hr_value;
	/** Number of RR intervals. */
	uint8_t rr_intervals_cnt;
	/** RR intervals. */
	uint16_t rr_intervals[BLE_HRS_C_RR_INTERVALS_MAX_CNT];
} ble_hrm_t;

/**
 * @brief Structure containing the handles related to the Heart Rate Service found on the peer.
 */
typedef struct {
	/** Handle of the CCCD of the Heart Rate Measurement characteristic. */
	uint16_t hrm_cccd_handle;
	/** Handle of the Heart Rate Measurement characteristic, as provided by the SoftDevice. */
	uint16_t hrm_handle;
} hrs_db_t;

/**
 * @brief Heart Rate Event structure.
 */
struct ble_hrs_c_evt {
	/** Type of the event. */
	ble_hrs_c_evt_type_t evt_type;
	/** Connection handle on which the Heart Rate service was discovered on the peer device. */
	uint16_t conn_handle;
	union {
		/** Handles related to the Heart Rate, found on the peer device.
		This is filled if the evt_type is @ref
		BLE_HRS_C_EVT_DISCOVERY_COMPLETE.*/
		hrs_db_t peer_db;
		/** Heart Rate Measurement received. This is filled if the evt_type
		is @ref BLE_HRS_C_EVT_HRM_NOTIFICATION. */
		ble_hrm_t hrm;
	} params;
};

/** @} */

/**
 * @defgroup hrs_c_types Types
 * @{
 */

/** Forward declaration of the ble_bas_t type. */
struct ble_hrs_c;

/**
 * @brief   Event handler type.
 *
 * @details This is the type of the event handler that is to be provided by the application
 *          of this module to receive events.
 */
typedef void (*ble_hrs_c_evt_handler_t)(struct ble_hrs_c *p_ble_hrs_c, struct ble_hrs_c_evt *p_evt);

/** @} */

/**
 * @brief Heart rate service configuration.
 */
struct ble_hrs_config {
	/**
	 * @brief Heart rate service event handler.
	 */
	ble_hrs_c_evt_handler_t evt_handler;
	/**
	 * @brief If sensor contact detection is to be supported or not.
	 */
	bool is_sensor_contact_supported;
	/**
	 * @brief Initial value of the Body Sensor Location characteristic, if not @c NULL.
	 */
	uint8_t *body_sensor_location;
	/**
	 * @brief Security requirement for writing the heart rate monitor characteristic CCCD.
	 */
	ble_gap_conn_sec_mode_t hrm_cccd_wr_sec;
	/**
	 * @brief Security requirement for reading the body sensor location characteristic value.
	 */
	ble_gap_conn_sec_mode_t bsl_rd_sec;
};

/**
 * @addtogroup hrs_c_structs
 * @{
 */

/**
 * @brief Heart Rate Client structure.
 */
struct ble_hrs_c {
	/** Connection handle, as provided by the SoftDevice. */
	uint16_t conn_handle;
	/** Handles related to HRS on the peer. */
	hrs_db_t peer_hrs_db;
	/** Application event handler to be called when there
	is an event related to the Heart Rate Service. */
	ble_hrs_c_evt_handler_t evt_handler;
	/** Function to be called in case of an error. */
	ble_srv_error_handler_t error_handler;
	/** Pointer to the BLE GATT Queue instance. */
	struct ble_gq *p_gatt_queue;
};

/**
 * @brief Heart Rate Client initialization structure.
 */
struct ble_hrs_c_config {
	/** Event handler to be called by the Heart Rate Client module when
	there is an event related to the Heart Rate Service. */
	ble_hrs_c_evt_handler_t evt_handler;
	/** Function to be called in case of an error. */
	ble_srv_error_handler_t error_handler;
	/** Pointer to the BLE GATT Queue instance. */
	struct ble_gq *p_gatt_queue;
};

/** @} */

/**
 * @defgroup hrs_c_functions Functions
 * @{
 */

/**
 * @brief     Function for initializing the Heart Rate Client module.
 *
 * @details   This function registers with the Database Discovery module for the Heart Rate Service.
 *		   	  The module looks for the presence of a Heart Rate Service instance at the
 *peer when a discovery is started.
 *
 * @param[in] p_ble_hrs_c      Pointer to the Heart Rate Client structure.
 * @param[in] p_ble_hrs_c_init Pointer to the Heart Rate initialization structure that contains
 *                             the initialization information.
 *
 * @retval    NRF_SUCCESS On successful initialization.
 * @retval    err_code    Otherwise, this function propagates the error code returned by the
 *Database Discovery module API
 *                        @ref ble_db_discovery_evt_register.
 */
uint32_t ble_hrs_c_init(struct ble_hrs_c *p_ble_hrs_c, struct ble_hrs_c_config *p_ble_hrs_c_init);

/**
 * @brief     Function for handling BLE events from the SoftDevice.
 *
 * @details   This function handles the BLE events received from the SoftDevice. If a BLE
 *            event is relevant to the Heart Rate Client module, the function uses the event's data
 * to update interval variables and, if necessary, send events to the application.
 *
 * @param[in] p_ble_evt     Pointer to the BLE event.
 * @param[in] p_context     Pointer to the Heart Rate Client structure.
 */
void ble_hrs_c_on_ble_evt(ble_evt_t const *p_ble_evt, void *p_context);

/**
 * @brief   Function for requesting the peer to start sending notification of Heart Rate
 *          Measurement.
 *
 * @details This function enables notification of the Heart Rate Measurement at the peer
 *          by writing to the CCCD of the Heart Rate Measurement characteristic.
 *
 * @param   p_ble_hrs_c Pointer to the Heart Rate Client structure.
 *
 * @retval  NRF_SUCCESS If the SoftDevice is requested to write to the CCCD of the peer.
 * @retval	err_code	Otherwise, this function propagates the error code returned
 *                      by the SoftDevice API @ref sd_ble_gattc_write.
 */
uint32_t ble_hrs_c_hrm_notif_enable(struct ble_hrs_c *p_ble_hrs_c);

/**
 * @brief     Function for handling events from the Database Discovery module.
 *
 * @details   Call this function when you get a callback event from the Database Discovery module.
 *            This function handles an event from the Database Discovery module and determines
 *            whether it relates to the discovery of Heart Rate Service at the peer. If it does, the
 * function calls the application's event handler to indicate that the Heart Rate Service was
 *            discovered at the peer. The function also populates the event with service-related
 *            information before providing it to the application.
 *
 * @param[in] p_ble_hrs_c Pointer to the Heart Rate Client structure instance for associating the
 * link.
 * @param[in] p_evt Pointer to the event received from the Database Discovery module.
 *
 */
void ble_hrs_on_db_disc_evt(struct ble_hrs_c *p_ble_hrs_c,
			    const struct ble_db_discovery_evt *p_evt);

/**
 * @brief     Function for assigning handles to an instance of hrs_c.
 *
 * @details   Call this function when a link has been established with a peer to
 *            associate the link to this instance of the module. This association makes it
 *            possible to handle several links and associate each link to a particular
 *            instance of this module. The connection handle and attribute handles are
 *            provided from the discovery event @ref BLE_HRS_C_EVT_DISCOVERY_COMPLETE.
 *
 * @param[in] p_ble_hrs_c        Pointer to the Heart Rate Client structure instance for associating
 * the link.
 * @param[in] conn_handle        Connection handle to associate with the given Heart Rate Client
 * Instance.
 * @param[in] p_peer_hrs_handles Attribute handles for the HRS server you want this HRS_C client to
 *                               interact with.
 */
uint32_t ble_hrs_c_handles_assign(struct ble_hrs_c *p_ble_hrs_c, uint16_t conn_handle,
				  const hrs_db_t *p_peer_hrs_handles);

/** @} */ // End tag for Function group.

#ifdef __cplusplus
}
#endif

#endif // BLE_HRS_C_H__

/** @} */ // End tag for the file.
