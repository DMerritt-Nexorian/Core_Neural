/**
 * @file invariant_validation.c
 * @brief Numerical validation demonstrating exponential decay of perturbations under contractive stability.
 * @note MISRA-C:2012 / ISO C18 compliant.
 */

#include "core_neural_types.h"
#include "core_neural_kernels.h"
#include "safety_invariants.h"
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

int main(void) {
    printf("====================================================================\n");
    printf("            CORE_NEURAL CONTRACTIVE INVARIANT VALIDATION\n");
    printf("====================================================================\n");

    /* Initialize two decoder states: actual and perturbed */
    DecoderState actual_state;
    DecoderState perturbed_state;
    DecoderConfig config;

    config.snn_decay = 0.8f;
    config.snn_threshold = 1.0f;
    config.dt = 0.001f;      /* 1 ms timestep */
    config.stability_c = 10.0f; /* c factor for contractive decay rate */

    /* Configure a stable transition matrix A = 0.85 * Identity */
    for (uint32_t i = 0U; i < STATE_DIM; ++i) {
        for (uint32_t j = 0U; j < STATE_DIM; ++j) {
            if (i == j) {
                config.A[i][j] = 0.85f;
            } else {
                config.A[i][j] = 0.0f;
            }
        }
    }

    /* Configure B matrix (all 0 for this isolated perturbation test) */
    for (uint32_t i = 0U; i < STATE_DIM; ++i) {
        for (uint32_t j = 0U; j < SNN_NEURONS; ++j) {
            config.B[i][j] = 0.0f;
        }
    }

    /* Initialize states */
    core_neural_init(&actual_state, &config);
    core_neural_init(&perturbed_state, &config);

    /* Set some initial kinematic trajectory value to actual */
    actual_state.traj.x[0] = 1.0f;
    actual_state.traj.x[1] = 2.0f;
    actual_state.traj.x[2] = 3.0f;
    actual_state.traj.x[3] = 4.0f;

    /* Add a large perturbation to the perturbed state */
    perturbed_state.traj.x[0] = actual_state.traj.x[0] + 5.0f;
    perturbed_state.traj.x[1] = actual_state.traj.x[1] - 4.0f;
    perturbed_state.traj.x[2] = actual_state.traj.x[2] + 3.0f;
    perturbed_state.traj.x[3] = actual_state.traj.x[3] - 2.0f;

    /* Compute initial perturbation vector and L2 norm */
    float32_t init_diff[STATE_DIM];
    for (uint32_t i = 0U; i < STATE_DIM; ++i) {
        init_diff[i] = perturbed_state.traj.x[i] - actual_state.traj.x[i];
    }
    const float32_t initial_perturbation_norm = tensor_vec_l2_norm(init_diff, STATE_DIM);

    printf("[STABILITY] Initial perturbation norm (t=0): %f\n", (double)initial_perturbation_norm);

    float32_t prev_norm = initial_perturbation_norm;
    bool mathematical_proof_verified = true;

    /* Simulate 50 steps of discrete-time evolution to watch perturbation decay */
    uint32_t steps = 50U;
    for (uint32_t step = 1U; step <= steps; ++step) {
        float32_t x_prev[STATE_DIM];
        float32_t xp_prev[STATE_DIM];

        for (uint32_t i = 0U; i < STATE_DIM; ++i) {
            x_prev[i] = actual_state.traj.x[i];
            xp_prev[i] = perturbed_state.traj.x[i];
        }

        /* Update both trajectories */
        float32_t raw_lfp[OBS_DIM];
        for (uint32_t i = 0U; i < OBS_DIM; ++i) {
            raw_lfp[i] = 0.0f;
        }

        core_neural_decode_step(&actual_state, &config, raw_lfp);
        core_neural_decode_step(&perturbed_state, &config, raw_lfp);

        /* Compute current step difference and norm */
        float32_t current_diff[STATE_DIM];
        for (uint32_t i = 0U; i < STATE_DIM; ++i) {
            current_diff[i] = perturbed_state.traj.x[i] - actual_state.traj.x[i];
        }
        const float32_t current_norm = tensor_vec_l2_norm(current_diff, STATE_DIM);

        /* Verify strict monotonic decreasing under contractive condition */
        const bool step_is_stable = verify_contractive_stability(
            xp_prev, x_prev,
            perturbed_state.traj.x, actual_state.traj.x,
            config.dt, config.stability_c
        );

        if (!step_is_stable || (current_norm >= prev_norm)) {
            mathematical_proof_verified = false;
        }

        prev_norm = current_norm;

        /* Print decay history every 10 steps */
        if ((step % 10U) == 0U) {
            const double theoretical_bound = (double)initial_perturbation_norm * pow(1.0 - (double)(config.stability_c * config.dt), (double)step);
            printf("[STABILITY] Step %2u: Norm = %10.6f (Theoretical Bound <= %10.6f)\n",
                   step, (double)current_norm, theoretical_bound);
        }
    }

    /* Final check: Perturbation should converge to virtually zero */
    printf("[STABILITY] Final perturbation norm (t=50): %f\n", (double)prev_norm);

    const float32_t decay_ratio = prev_norm / initial_perturbation_norm;
    printf("[STABILITY] Total Decay Ratio: %e (expected < 1e-3)\n", (double)decay_ratio);

    if (mathematical_proof_verified && (decay_ratio < 1e-3f)) {
        printf("====================================================================\n");
        printf("[SUCCESS] Contractive stability exponentially verified.\n");
        printf("====================================================================\n");
        return 0;
    } else {
        printf("[FAIL] Contractive stability was not verified.\n");
        return 1;
    }
}
