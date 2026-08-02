/**
 * @file safety_invariants.h
 * @brief Mathematical invariant enforcement, bounds checking, and projections.
 * @note MISRA-C:2012 / ISO C18 compliant.
 */

#ifndef SAFETY_INVARIANTS_H
#define SAFETY_INVARIANTS_H

#include "core_neural_types.h"

/**
 * @brief Verifies that the discrete-time system state update adheres to the contractive
 *        stability requirement: d/dt ||delta x(t)|| <= -c ||delta x(t)||.
 *        Discretized as: ||next_x1 - next_x2|| <= (1 - c * dt) * ||x1 - x2||
 * @param[in] x1       First state vector at step t (size STATE_DIM).
 * @param[in] x2       Second state vector at step t (size STATE_DIM).
 * @param[in] next_x1  First state vector at step t+1 (size STATE_DIM).
 * @param[in] next_x2  Second state vector at step t+1 (size STATE_DIM).
 * @param[in] dt       Discrete-time step size (seconds).
 * @param[in] c        Stability contractive coefficient (c > 0).
 * @return true if contractive stability holds, false otherwise.
 */
bool verify_contractive_stability(const float32_t *const x1, const float32_t *const x2,
                                  const float32_t *const next_x1, const float32_t *const next_x2,
                                  float32_t dt, float32_t c);

/**
 * @brief Enforces weight matrix constraints by projecting weights onto the convex
 *        constraint set C, defined by a Frobenius norm limit.
 *        W_projected = W * min(1, max_norm / ||W||_Frobenius)
 * @param[in,out] matrix    Pointer to the flat matrix buffer.
 * @param[in]     rows      Number of rows.
 * @param[in]     cols      Number of columns.
 * @param[in]     max_norm  Maximum allowed Frobenius norm of the matrix.
 */
void project_weight_matrix(float32_t *const matrix, uint32_t rows, uint32_t cols, float32_t max_norm);

/**
 * @brief Verifies whether the subdermal system operates within safe physical and telemetry boundaries.
 *        - Latency <= 5.0 ms
 *        - Packet loss <= 0.1% (0.001)
 *        - Tissue impedance < 50.0 kOhm across all OBS_DIM channels
 * @param[in] status  Pointer to the active health and status structure.
 * @return true if all safety boundaries are met, false otherwise.
 */
bool verify_system_boundaries(const SystemHealthStatus *const status);

#endif /* SAFETY_INVARIANTS_H */
