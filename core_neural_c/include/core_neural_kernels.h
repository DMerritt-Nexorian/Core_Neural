/**
 * @file core_neural_kernels.h
 * @brief Neural processing and state-space decoding kernels.
 * @note MISRA-C:2012 / ISO C18 compliant.
 */

#ifndef CORE_NEURAL_KERNELS_H
#define CORE_NEURAL_KERNELS_H

#include "core_neural_types.h"

/**
 * @brief Initialize the neural decoder state.
 * @param[out] state   Pointer to the decoder state to initialize.
 * @param[in]  config  Pointer to the decoder configuration.
 */
void core_neural_init(DecoderState *const state, const DecoderConfig *const config);

/**
 * @brief Execute a single real-time decoding step.
 *        1. Update on-chip Neuromorphic SNN using incoming Local Field Potentials (LFPs) / raw signals.
 *        2. Update continuous-time kinematic state-space trajectory estimates based on SNN spikes.
 * @param[in,out] state    Pointer to the active decoder state.
 * @param[in]     config   Pointer to the decoder parameters/weights.
 * @param[in]     raw_lfp  Pointer to the raw observation signal (size OBS_DIM).
 */
void core_neural_decode_step(DecoderState *const state, const DecoderConfig *const config, const float32_t *const raw_lfp);

/* Statically-allocated matrix/vector math routines (from tensor_ops.c) */

/**
 * @brief Perform matrix-vector multiplication: y = A * x
 * @param[out] y       Result vector of size rows.
 * @param[in]  matrix  Input matrix of size rows x cols.
 * @param[in]  x       Input vector of size cols.
 * @param[in]  rows    Number of rows.
 * @param[in]  cols    Number of columns.
 * @return true if successful, false on bounds check failure.
 */
bool tensor_mat_vec_mult(float32_t *const y, const float32_t *const matrix, const float32_t *const x, uint32_t rows, uint32_t cols);

/**
 * @brief Add two vectors: result = a + b
 * @param[out] result  Output vector of size size.
 * @param[in]  a       First vector of size size.
 * @param[in]  b       Second vector of size size.
 * @param[in]  size    Vector length.
 * @return true if successful, false on bounds check failure.
 */
bool tensor_vec_add(float32_t *const result, const float32_t *const a, const float32_t *const b, uint32_t size);

/**
 * @brief Calculate the Euclidean (L2) norm of a vector.
 * @param[in]  vector  Input vector.
 * @param[in]  size    Vector size.
 * @return The L2 norm, or negative value on bounds error.
 */
float32_t tensor_vec_l2_norm(const float32_t *const vector, uint32_t size);

#endif /* CORE_NEURAL_KERNELS_H */
