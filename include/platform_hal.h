/**
 * @file platform_hal.h
 * @brief Hardware Abstraction Layer (HAL) interface for target silicon (ARM / RISC-V).
 * @note MISRA-C:2012 / ISO C18 compliant.
 */

#ifndef PLATFORM_HAL_H
#define PLATFORM_HAL_H

#include "core_neural_types.h"

/**
 * @brief Retrieve system clock timestamp in microseconds.
 *        Used to measure end-to-end decoding latency and assure real-time deadlines.
 * @return 64-bit microsecond counter.
 */
uint64_t platform_get_time_us(void);

/**
 * @brief Reads Local Field Potential (LFP) inputs from the subdermal graphene micro-thread electrode arrays.
 * @param[out] lfp_dest  Target destination buffer to write inputs (size OBS_DIM).
 * @param[in]  size      Number of channels to read (must be OBS_DIM).
 * @return true if raw LFP is read successfully, false otherwise.
 */
bool platform_read_lfp(float32_t *const lfp_dest, uint32_t size);

/**
 * @brief Dispatches the current kinematic state and telemetry packets to the sub-clavicular telemetry link.
 * @param[in] state   Current Decoder State.
 * @param[in] health  Current System Health Status.
 * @return true if packet was successfully serialized and transmitted without data loss.
 */
bool platform_send_telemetry(const DecoderState *const state, const SystemHealthStatus *const health);

/**
 * @brief Populates the current system health telemetry status (e.g., latency, packet loss, electrode impedance).
 * @param[out] health Target status struct to populate.
 */
void platform_get_system_health(SystemHealthStatus *const health);

#endif /* PLATFORM_HAL_H */
