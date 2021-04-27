/*
 * @file lcz_event_manager_file_handler.h
 * @brief Interface to the Event manager data handling functions.
 *
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LCZ_EVENT_MANAGER_FILE_HANDLER_H

#define LCZ_EVENT_MANAGER_FILE_HANDLER_H

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
/* This is used to indicate the status of the last log file creation request. */
typedef enum {
	LOG_FILE_STATUS_WAITING = 0,
	LOG_FILE_STATUS_PREPARING,
	LOG_FILE_STATUS_READY,
	LOG_FILE_STATUS_FAILED,
	LOG_FILE_STATUS_COUNT
} LogFileStatus_t;

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
/** @brief Initialises kernel objects
 *
 */
void lcz_event_manager_file_handler_initialise(void);

/** @brief Adds an event to the event log.
 *
 *  @param [in]sensorEventType - The sensor event type.
 *  @param [in]pSensorEventData - The data associated with the event.
 *  @param [in]timestamp - The timestamp associated with the event.
 */
void lcz_event_manager_file_handler_add_event(
	SensorEventType_t sensorEventType, SensorEventData_t *pSensorEventData,
	uint32_t timestamp);

/** @brief Builds an event log file for reading over the device user
 *         interfaces.
 *
 *  @param [out]absFilePath - The absolute file path where the file was created
 *  @param [out]file_size - The size of the file in bytes.
 *  @param [in]is_running - Flag used to skip triggering the background file
 *                          creation for debug contexts.
 *  @return Non-zero failure code, 0 on success.
 */
int lcz_event_manager_file_handler_build_file(uint8_t *absFilePath,
					      uint32_t *file_size,
					      bool is_running);

/** @brief Deletes the last created output log file.
 *  @return Non-zero failure code, 0 on success.
 */
int lcz_event_manager_file_handler_delete_file(void);

/** @brief Gets the count of events at the passed timestamp.
 *
 *  @param [in]timestamp - The timestamp where to look for events.
 *  @param [in]index - The zero based sub-index of the event.
 *  @param [out]count - The number of events that reside at this timestamp.
 *  @return Pointer to the requested event if found, NULL otherwise.
 */
SensorEvent_t *lcz_event_manager_file_handler_get_indexed_event_at_timestamp(
	uint32_t timestamp, uint16_t index, uint16_t *count);

/** @brief Gets the status of the last create log file request.
 *
 *  @return Details of the last log file create request.
 */
LogFileStatus_t lcz_event_manager_file_handler_get_log_file_status(void);

#endif /* ifdef LCZ_EVENT_MANAGER_FILE_HANDLER_H */