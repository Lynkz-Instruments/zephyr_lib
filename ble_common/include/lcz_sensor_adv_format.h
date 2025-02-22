/**
 * @file lcz_sensor_adv_format.h
 * @brief Advertisement format for Laird BT sensors
 *
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __LCZ_SENSOR_ADV_FORMAT_H__
#define __LCZ_SENSOR_ADV_FORMAT_H__

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr/types.h>
#include <bluetooth/bluetooth.h>

#include "lcz_sensor_event.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Common                                                                     */
/******************************************************************************/
#define SENSOR_ADDR_STR_SIZE 13
#define SENSOR_ADDR_STR_LEN (SENSOR_ADDR_STR_SIZE - 1)

#define SENSOR_NAME_MAX_SIZE 32
#define SENSOR_NAME_MAX_STR_LEN (SENSOR_NAME_MAX_SIZE - 1)

#define LAIRD_CONNECTIVITY_MANUFACTURER_SPECIFIC_COMPANY_ID1 0x0077
#define LAIRD_CONNECTIVITY_MANUFACTURER_SPECIFIC_COMPANY_ID2 0x00E4

#define LYNKZ_INSTRUMENT_MANUFACTURER_SPECIFIC_COMPANY_ID1 0x6666
#define LYNKZ_INSTRUMENT_MANUFACTURER_SPECIFIC_COMPANY_ID2 0x6667

/* clang-format off */
#define RESERVED_AD_PROTOCOL_ID               0x0000
#define BTXXX_1M_PHY_AD_PROTOCOL_ID           0x0001
#define BTXXX_CODED_PHY_AD_PROTOCOL_ID        0x0002
#define BTXXX_1M_PHY_RSP_PROTOCOL_ID          0x0003
#define RS1XX_BOOTLOADER_AD_PROTOCOL_ID       0x0004
#define RS1XX_BOOTLOADER_RSP_PROTOCOL_ID      0x0005
#define RS1XX_SENSOR_AD_PROTOCOL_ID           0x0006
#define RS1XX_SENSOR_RSP_PROTOCOL_ID          0x0007
#define BTXXX_DM_1M_PHY_AD_PROTOCOL_ID        0x0008
#define BTXXX_DM_CODED_PHY_AD_PROTOCOL_ID     0x0009
#define BTXXX_DM_ENC_CODED_PHY_AD_PROTOCOL_ID 0x000A
#define BTXXX_DM_1M_PHY_RSP_PROTOCOL_ID       0x000B
#define LYNKZ_1M_PHY_AD_PROTOCOL_ID 	      0x000C
#define LYNKZ_1M_PHY_RSP_PROTOCOL_ID 	      0x000D
#define CT_TRACKER_AD_PROTOCOL_ID             0xFF81
#define CT_GATEWAY_AD_PROTOCOL_ID             0xFF82
#define CT_DATA_DOWNLOAD_AD_PROTOCOL_ID       0xFF83
/* clang-format on */

#define ADV_FORMAT_HW_VERSION(major, minor) ((uint8_t)(((((uint32_t)(major)) << 3) & 0x000000F8) | ((((uint32_t)(minor)) << 0 ) & 0x00000007))

#define ADV_FORMAT_HW_VERSION_GET_MAJOR(x) ((x & 0x000000F8) >> 3)
#define ADV_FORMAT_HW_VERSION_GET_MINOR(x) ((x & 0x00000007) >> 0)

/* clang-format off */
#define SENSOR_ADV_LENGTH_MANUFACTURER_SPECIFIC            24
#define SENSOR_ADV_LENGTH_MANUFACTURER_SPECIFIC_EXTENDED   35
#define SENSOR_MAX_ADV_LENGTH                              31
#define SENSOR_MAX_ADV_LENGTH_EXTENDED                     67
#define SENSOR_MAX_RSP_LENGTH                              31
#define SENSOR_MAX_NAME_LENGTH                             12
#define SENSOR_MAX_NAME_LENGTH_EXTENDED                    23
#define SENSOR_RSP_LENGTH_MANUFACTURER_SPECIFIC            13
/* clang-format on */

/* clang-format off */
#define BT510_PRODUCT_ID     0
#define BT6XX_PRODUCT_ID     1
#define BT6XX_DM_PRODUCT_ID  2
#define INVALID_PRODUCT_ID   0xFFFF
/* clang-format on */

/* clang-format off */
#define BTXXX_DEFAULT_NETWORK_ID  0x0000
#define CT_DEFAULT_NETWORK_ID     0xFFFF
/* clang-format on */

/* Format of the Manufacturer Specific Data (MSD) using 1M PHY of TAG.
 * Format of the 1st chunk of MSD when using coded PHY.
 */
struct LynkzSensorAdEvent {
	uint16_t companyId;
	uint16_t protocolId;
	uint16_t productId;
} __packed;

struct LynkzSensorRspEvent {
	uint16_t companyId;
	uint16_t protocolId;
	uint8_t packetIndex;
	uint8_t event_type;
	uint8_t data_size;
	uint8_t data[20];
	uint16_t crc;

} __packed;

/* Format of the Manufacturer Specific Data (MSD) using 1M PHY.
 * Format of the 1st chunk of MSD when using coded PHY.
 */
struct LczSensorAdEvent {
	uint16_t companyId;
	uint16_t protocolId;
	uint16_t networkId;
	uint16_t flags;
	bt_addr_t addr;
	uint8_t recordType;
	uint16_t id;
	uint32_t epoch;
	SensorEventData_t data;
	uint8_t resetCount;
} __packed;

/* Format of the response payload for 1M PHY.
 * This is the second chunk of the extended advertisement data
 * when using the coded PHY.
 */
struct LczSensorRsp {
	uint16_t productId;
	uint8_t firmwareVersionMajor;
	uint8_t firmwareVersionMinor;
	uint8_t firmwareVersionPatch;
	uint8_t firmwareType;
	uint8_t configVersion;
	uint8_t bootloaderVersionMajor;
	uint8_t bootloaderVersionMinor;
	uint8_t bootloaderVersionPatch;
	uint8_t hardwareVersion; /* major + minor stuffed into one byte */
} __packed;

/* Format of the Manufacturer Specific Data using 1M PHY in Scan Response */
struct LczSensorRspWithHeader {
	uint16_t companyId;
	uint16_t protocolId;
	struct LczSensorRsp rsp;
} __packed;

/* Format of the Manufacturer Specific Data for Coded PHY */
struct LczSensorAdCoded {
	struct LczSensorAdEvent ad;
	struct LczSensorRsp rsp;
} __packed;

/* Format of the Device Management Manufacturer Specific Data (MSD)
 * using 1M PHY or the coded PHY (unencrypted)
 */
struct LczSensorDMUnencrAd {
	uint16_t companyId;
	uint16_t protocolId;
	uint16_t networkId;
	uint16_t productId;
	uint16_t flags;
	bt_addr_t addr;
} __packed;

/* Format of the Device Management Manufacturer Specifc Data (MSD)
 * using the coded PHY (encrypted)
 */
struct LczSensorDMEncrAd {
	uint16_t companyId;
	uint16_t protocolId;
	uint16_t networkId;
	uint16_t productId;
	uint16_t flags;
	bt_addr_t addr;
	uint16_t mic;
	uint32_t epoch;
	uint16_t id;
	/* Below this line, the data is encrypted */
	uint8_t recordType;
	SensorEventData_t data;
} __packed;

/* Contact Tracing */
struct LczContactTracingAd {
	uint16_t companyId;
	uint16_t protocolId;
	uint16_t networkId;
	uint16_t flags;
	bt_addr_t addr;
	uint8_t recordType;
	uint8_t deviceType;
	uint32_t epoch;
	int8_t txPower;
	uint8_t motionMagnitude;
	uint8_t modelId;
	uint8_t reserved_1;
	uint8_t reserved_2;
	uint8_t reserved_3;
} __packed;
BUILD_ASSERT(sizeof(struct LczContactTracingAd) == 26, "Unexpected ad size");

/* clang-format off */
typedef struct LczSensorAdEvent       LczSensorAdEvent_t;
typedef struct LczSensorRsp           LczSensorRsp_t;
typedef struct LczSensorRspWithHeader LczSensorRspWithHeader_t;
typedef struct LczSensorAdCoded       LczSensorAdCoded_t;
typedef struct LczSensorAdCoded       LczSensorAdExt_t;
typedef struct LczSensorDMUnencrAd    LczSensorDMUnencrAd_t;
typedef struct LczSensorDMEncrAd      LczSensorDMEncrAd_t;
typedef struct LczContactTracingAd    LczContactTracingAd_t;
typedef struct LynkzSensorAdEvent	  LynkzSensorAdEvent_t;
typedef struct LynkzSensorRspEvent	  LynkzSensorRspEvent_t;
/* clang-format on */

/*
 * This is the format for the 1M PHY.
 */
#define LCZ_SENSOR_MSD_AD_FIELD_LENGTH 0x1b
#define LCZ_SENSOR_MSD_AD_PAYLOAD_LENGTH (LCZ_SENSOR_MSD_AD_FIELD_LENGTH - 1)
BUILD_ASSERT(sizeof(LczSensorAdEvent_t) == LCZ_SENSOR_MSD_AD_PAYLOAD_LENGTH,
	     "Advertisement data size mismatch (check packing)");

#define LCZ_SENSOR_MSD_RSP_FIELD_LENGTH 0x10
#define LCZ_SENSOR_MSD_RSP_PAYLOAD_LENGTH (LCZ_SENSOR_MSD_RSP_FIELD_LENGTH - 1)
BUILD_ASSERT(sizeof(LczSensorRspWithHeader_t) ==
		     LCZ_SENSOR_MSD_RSP_PAYLOAD_LENGTH,
	     "Scan Response size mismatch (check packing)");

/*
 * Coded PHY
 */
#define LCZ_SENSOR_MSD_CODED_FIELD_LENGTH 0x26
#define LCZ_SENSOR_MSD_CODED_PAYLOAD_LENGTH                                    \
	(LCZ_SENSOR_MSD_CODED_FIELD_LENGTH - 1)
BUILD_ASSERT(sizeof(LczSensorAdCoded_t) == LCZ_SENSOR_MSD_CODED_PAYLOAD_LENGTH,
	     "Coded advertisement size mismatch (check packing)");

/*
 * DM Advertisements
 */
#define LCZ_SENSOR_MSD_DM_UNENCR_FIELD_LENGTH 17
#define LCZ_SENSOR_MSD_DM_UNENCR_PAYLOAD_LENGTH \
	(LCZ_SENSOR_MSD_DM_UNENCR_FIELD_LENGTH - 1)
BUILD_ASSERT(sizeof(LczSensorDMUnencrAd_t) == LCZ_SENSOR_MSD_DM_UNENCR_PAYLOAD_LENGTH,
	     "DM unencrypted advertisement size mismatch (check packing)");

#define LCZ_SENSOR_MSD_DM_ENCR_FIELD_LENGTH 30
#define LCZ_SENSOR_MSD_DM_ENCR_PAYLOAD_LENGTH \
	(LCZ_SENSOR_MSD_DM_ENCR_FIELD_LENGTH - 1)
BUILD_ASSERT(sizeof(LczSensorDMEncrAd_t) == LCZ_SENSOR_MSD_DM_ENCR_PAYLOAD_LENGTH,
	     "DM encrypted advertisement size mismatch (check packing)");

/* Bytes used to differentiate advertisement types/sensors. */
#define LCZ_SENSOR_AD_HEADER_SIZE 4
extern const uint8_t BTXXX_AD_HEADER[LCZ_SENSOR_AD_HEADER_SIZE];
extern const uint8_t LYNKZ_AD_HEADER[LCZ_SENSOR_AD_HEADER_SIZE];
extern const uint8_t LYNKZ_RSP_HEADER[LCZ_SENSOR_AD_HEADER_SIZE];
extern const uint8_t BT5XX_RSP_HEADER[LCZ_SENSOR_AD_HEADER_SIZE];
extern const uint8_t BT6XX_RSP_HEADER[LCZ_SENSOR_AD_HEADER_SIZE];
extern const uint8_t BTXXX_CODED_HEADER[LCZ_SENSOR_AD_HEADER_SIZE];
extern const uint8_t BTXXX_DM_1M_HEADER[LCZ_SENSOR_AD_HEADER_SIZE];
extern const uint8_t BTXXX_DM_CODED_HEADER[LCZ_SENSOR_AD_HEADER_SIZE];
extern const uint8_t BTXXX_DM_ENC_CODED_HEADER[LCZ_SENSOR_AD_HEADER_SIZE];
extern const uint8_t CT_TRACKER_AD_HEADER[LCZ_SENSOR_AD_HEADER_SIZE];
extern const uint8_t CT_GATEWAY_AD_HEADER[LCZ_SENSOR_AD_HEADER_SIZE];
extern const uint8_t CT_DATA_DOWNLOAD_AD_HEADER[LCZ_SENSOR_AD_HEADER_SIZE];

/* Format is the same for all versions,
 * but they are needed for backward compatiblity.
 */
enum ContactTracingAdRecordType {
	CT_ADV_REC_TYPE_V00 = 0x00,
	CT_ADV_REC_TYPE_V10 = 0x10,
	CT_ADV_REC_TYPE_V11 = 0x11
};

enum ContactTracingAdFlags {
	CT_ADV_FLAGS_HAS_EPOCH_TIME = BIT(0),
	CT_ADV_FLAGS_HAS_LOG_DATA = BIT(1),
	CT_ADV_FLAGS_HAS_MOTION = BIT(2),
	CT_ADV_FLAGS_LOW_BATTERY = BIT(3),
	CT_ADV_FLAGS_DATALOG_FULL = BIT(4)
};

enum LczSensorModelId {
	LCZ_SENSOR_MODEL_ID_BT510 = 0x00,
	LCZ_SENSOR_MODEL_ID_BL654_DVK = 0x10,
	LCZ_SENSOR_MODEL_ID_BL653_DVK = 0x20,
	LCZ_SENSOR_MODEL_ID_BT710 = 0x30,
	LCZ_SENSOR_MODEL_ID_MG100 = 0x40,
	LCZ_SENSOR_MODEL_ID_IG60 = 0x50
};

#ifdef __cplusplus
}
#endif

#endif /* __LCZ_SENSOR_ADV_FORMAT_H__ */
