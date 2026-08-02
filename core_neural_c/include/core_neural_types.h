/**
 * @file core_neural_types.h
 * @brief Fixed-width types, tensor dimensions, and config limits for Core_Neural.
 * @note MISRA-C:2012 / ISO C18 compliant. No dynamic allocation.
 */

#ifndef CORE_NEURAL_TYPES_H
#define CORE_NEURAL_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/* Fixed-width floating-point representation as per MISRA-C:2012 recommendations */
typedef float float32_t;
typedef double float64_t;

/* System Dimensions */
#define STATE_DIM    4U   /**< Kinematic trajectory state size (x_pos, y_pos, x_vel, y_vel) */
#define OBS_DIM      64U  /**< Input observation dimension (64 subdermal electrode channels) */
#define SNN_NEURONS  32U  /**< Localized on-chip neuromorphic SNN neuron count */

/* Operational Boundaries & Failure Metrics */
#define MAX_ACCEPTABLE_LATENCY_MS       5.0f
#define MAX_ACCEPTABLE_LOSS_RATE        0.001f
#define MAX_ACCEPTABLE_IMPEDANCE_KOHM   50.0f
#define MAX_GLIAL_SCAR_RADIUS_MICRONS   150.0f

/**
 * @brief Spiking Neural Network (SNN) state representation.
 */
typedef struct {
    float32_t v[SNN_NEURONS];      /**< Membrane potential vector */
    uint8_t spikes[SNN_NEURONS];   /**< Binary spike output vector (0 or 1) */
} SNNState;

/**
 * @brief Kinematic trajectory decoding state.
 */
typedef struct {
    float32_t x[STATE_DIM];        /**< Trajectory state vector: [x_pos, y_pos, x_vel, y_vel] */
} TrajectoryState;

/**
 * @brief Complete BCI Decoder State.
 */
typedef struct {
    SNNState snn;
    TrajectoryState traj;
} DecoderState;

/**
 * @brief Neural Decoder Parameters and Weight Matrices.
 */
typedef struct {
    float32_t A[STATE_DIM][STATE_DIM];         /**< Trajectory state transition matrix */
    float32_t B[STATE_DIM][SNN_NEURONS];       /**< State control input mapping matrix from SNN */
    float32_t W_snn[SNN_NEURONS][OBS_DIM];     /**< SNN input synaptic weight matrix */
    float32_t snn_decay;                       /**< LIF membrane potential decay coefficient */
    float32_t snn_threshold;                   /**< LIF neuron firing threshold */
    float32_t dt;                              /**< Timestep increment (seconds) */
    float32_t stability_c;                     /**< Contractive stability coefficient c > 0 */
} DecoderConfig;

/**
 * @brief Telemetry & System Health Status structure.
 */
typedef struct {
    float32_t packet_loss_rate;                /**< Measured wireless telemetry packet loss rate */
    float32_t current_latency_ms;              /**< Measured end-to-end feedback loop latency */
    float32_t channel_impedance[OBS_DIM];      /**< Measured subdermal micro-thread impedances */
} SystemHealthStatus;

#endif /* CORE_NEURAL_TYPES_H */
