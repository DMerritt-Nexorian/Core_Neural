/**
 * @file core_neural_kernels.c
 * @brief Implementations of core neural processing and state-space decoding.
 * @note MISRA-C:2012 / ISO C18 compliant.
 */

#include "core_neural_kernels.h"
#include <stddef.h>

/**
 * @brief Initialize the neural decoder state.
 */
void core_neural_init(DecoderState *const state, const DecoderConfig *const config) {
    if ((state != NULL) && (config != NULL)) {
        /* Initialize Trajectory state to zero */
        for (uint32_t i = 0U; i < STATE_DIM; ++i) {
            state->traj.x[i] = 0.0f;
        }

        /* Initialize SNN membrane potentials and spike outputs to zero */
        for (uint32_t i = 0U; i < SNN_NEURONS; ++i) {
            state->snn.v[i] = 0.0f;
            state->snn.spikes[i] = 0U;
        }
    }
}

/**
 * @brief Execute a single real-time decoding step (SNN + Kinematic state-space).
 */
void core_neural_decode_step(DecoderState *const state, const DecoderConfig *const config, const float32_t *const raw_lfp) {
    if ((state != NULL) && (config != NULL) && (raw_lfp != NULL)) {
        float32_t spikes_f[SNN_NEURONS];

        /* --- STAGE 1: ON-CHIP NEUROMORPHIC SNN HARDWARE ACCELERATION MOCK --- */
        for (uint32_t i = 0U; i < SNN_NEURONS; ++i) {
            /* Accumulate inputs from LFP across observation channels with weights */
            float32_t synaptic_input = 0.0f;
            for (uint32_t j = 0U; j < OBS_DIM; ++j) {
                synaptic_input += config->W_snn[i][j] * raw_lfp[j];
            }

            /* Update Leaky Integrate-and-Fire (LIF) membrane potential */
            state->snn.v[i] = (state->snn.v[i] * config->snn_decay) + synaptic_input;

            /* Event-driven spike execution and threshold reset */
            if (state->snn.v[i] >= config->snn_threshold) {
                state->snn.spikes[i] = 1U;
                state->snn.v[i] = 0.0f; /* Reset membrane potential */
            } else {
                state->snn.spikes[i] = 0U;
            }

            /* Store spike state as float for state-space multiplication */
            spikes_f[i] = (float32_t)state->snn.spikes[i];
        }

        /* --- STAGE 2: KINEMATIC STATE-SPACE DECODING TRAJECTORY UPDATE --- */
        float32_t term_a[STATE_DIM];
        float32_t term_b[STATE_DIM];
        float32_t next_x[STATE_DIM];

        /* term_a = A * x_t */
        const bool ok_a = tensor_mat_vec_mult(term_a, (const float32_t *)config->A, state->traj.x, STATE_DIM, STATE_DIM);

        /* term_b = B * spikes */
        const bool ok_b = tensor_mat_vec_mult(term_b, (const float32_t *)config->B, spikes_f, STATE_DIM, SNN_NEURONS);

        /* next_x = term_a + term_b */
        const bool ok_sum = tensor_vec_add(next_x, term_a, term_b, STATE_DIM);

        /* Save updated trajectory state if all steps executed successfully */
        if (ok_a && ok_b && ok_sum) {
            for (uint32_t i = 0U; i < STATE_DIM; ++i) {
                state->traj.x[i] = next_x[i];
            }
        }
    }
}
