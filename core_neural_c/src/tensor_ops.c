/**
 * @file tensor_ops.c
 * @brief Statically-allocated, bounds-checked matrix and vector math routines.
 * @note MISRA-C:2012 / ISO C18 compliant.
 */

#include "core_neural_kernels.h"
#include <stddef.h>
#include <math.h>

/**
 * @brief Perform matrix-vector multiplication: y = A * x
 */
bool tensor_mat_vec_mult(float32_t *const y, const float32_t *const matrix, const float32_t *const x, uint32_t rows, uint32_t cols) {
    bool success = true;

    /* Strict input contract checking */
    if ((y == NULL) || (matrix == NULL) || (x == NULL)) {
        success = false;
    } else if ((rows == 0U) || (cols == 0U)) {
        success = false;
    } else {
        /* Clear target vector y */
        for (uint32_t r = 0U; r < rows; ++r) {
            y[r] = 0.0f;
        }

        /* Matrix vector product execution */
        for (uint32_t r = 0U; r < rows; ++r) {
            float32_t sum = 0.0f;
            for (uint32_t c = 0U; c < cols; ++c) {
                /* Safe offset calculation for flat matrix access */
                const uint32_t index = (r * cols) + c;
                sum += matrix[index] * x[c];
            }
            y[r] = sum;
        }
    }

    return success;
}

/**
 * @brief Add two vectors: result = a + b
 */
bool tensor_vec_add(float32_t *const result, const float32_t *const a, const float32_t *const b, uint32_t size) {
    bool success = true;

    /* Strict input contract checking */
    if ((result == NULL) || (a == NULL) || (b == NULL)) {
        success = false;
    } else if (size == 0U) {
        success = false;
    } else {
        for (uint32_t i = 0U; i < size; ++i) {
            result[i] = a[i] + b[i];
        }
    }

    return success;
}

/**
 * @brief Calculate the Euclidean (L2) norm of a vector.
 */
float32_t tensor_vec_l2_norm(const float32_t *const vector, uint32_t size) {
    float32_t norm = -1.0f; /* Sentinel value for error condition */

    if ((vector != NULL) && (size > 0U)) {
        float32_t sum_squares = 0.0f;
        for (uint32_t i = 0U; i < size; ++i) {
            sum_squares += vector[i] * vector[i];
        }

        /* Compute square root in IEEE-754 single precision */
        norm = sqrtf(sum_squares);
    }

    return norm;
}
