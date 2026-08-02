/**
 * @file safety_invariants.c
 * @brief Implementations of contractive stability, weight projections, and operational boundary checks.
 * @note MISRA-C:2012 / ISO C18 compliant.
 */

#include "safety_invariants.h"
#include "core_neural_kernels.h"
#include <stddef.h>
#include <math.h>

/**
 * @brief Verifies discretized contractive stability:
 *        ||next_x1 - next_x2|| <= (1 - c * dt) * ||x1 - x2||
 */
bool verify_contractive_stability(const float32_t *const x1, const float32_t *const x2,
                                  const float32_t *const next_x1, const float32_t *const next_x2,
                                  float32_t dt, float32_t c) {
    bool stable = false;

    if ((x1 != NULL) && (x2 != NULL) && (next_x1 != NULL) && (next_x2 != NULL) && (dt > 0.0f) && (c > 0.0f)) {
        float32_t diff_t[STATE_DIM];
        float32_t diff_t_next[STATE_DIM];

        /* Compute differences */
        for (uint32_t i = 0U; i < STATE_DIM; ++i) {
            diff_t[i] = x1[i] - x2[i];
            diff_t_next[i] = next_x1[i] - next_x2[i];
        }

        /* Calculate L2 norms */
        const float32_t norm_t = tensor_vec_l2_norm(diff_t, STATE_DIM);
        const float32_t norm_t_next = tensor_vec_l2_norm(diff_t_next, STATE_DIM);

        if ((norm_t >= 0.0f) && (norm_t_next >= 0.0f)) {
            const float32_t decay_factor = 1.0f - (c * dt);

            /* If decay_factor is negative, the discrete-time step is too large,
             * but we still check the mathematical inequality constraint. */
            const float32_t limit = decay_factor * norm_t;

            if (norm_t_next <= limit) {
                stable = true;
            }
        }
    }

    return stable;
}

/**
 * @brief Projects weight matrix onto convex set C via Frobenius norm scaling.
 */
void project_weight_matrix(float32_t *const matrix, uint32_t rows, uint32_t cols, float32_t max_norm) {
    if ((matrix != NULL) && (rows > 0U) && (cols > 0U) && (max_norm > 0.0f)) {
        float32_t sum_squares = 0.0f;
        const uint32_t total_elements = rows * cols;

        for (uint32_t i = 0U; i < total_elements; ++i) {
            sum_squares += matrix[i] * matrix[i];
        }

        const float32_t frob_norm = sqrtf(sum_squares);

        /* Apply projection constraint if the Frobenius norm exceeds limit */
        if (frob_norm > max_norm) {
            /* Prevent division-by-zero or extremely small norms */
            if (frob_norm > 1e-6f) {
                const float32_t scale = max_norm / frob_norm;
                for (uint32_t i = 0U; i < total_elements; ++i) {
                    matrix[i] *= scale;
                }
            }
        }
    }
}

/**
 * @brief Verifies that physical, clinical, and telemetry invariants are within spec.
 */
bool verify_system_boundaries(const SystemHealthStatus *const status) {
    bool within_boundaries = true;

    if (status == NULL) {
        within_boundaries = false;
    } else {
        /* Latency limit <= 5.0 ms */
        if (status->current_latency_ms > MAX_ACCEPTABLE_LATENCY_MS) {
            within_boundaries = false;
        }

        /* Packet loss rate <= 0.1% */
        if (status->packet_loss_rate > MAX_ACCEPTABLE_LOSS_RATE) {
            within_boundaries = false;
        }

        /* Electrode impedance check: must be < 50.0 kOhm for all channels */
        for (uint32_t i = 0U; i < OBS_DIM; ++i) {
            if (status->channel_impedance[i] >= MAX_ACCEPTABLE_IMPEDANCE_KOHM) {
                within_boundaries = false;
                break;
            }
        }
    }

    return within_boundaries;
}
