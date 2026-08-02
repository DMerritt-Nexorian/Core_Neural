/**
 * @file unit_tests.c
 * @brief Unit tests for matrix/vector operations, decoding logic, determinism, and exact outputs.
 * @note MISRA-C:2012 / ISO C18 compliant.
 */

#include "core_neural_types.h"
#include "core_neural_kernels.h"
#include "safety_invariants.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            printf("[FAIL] %s:%d: Assertion failed: %s\n", __FILE__, __LINE__, #condition); \
            return false; \
        } \
    } while (0)

#define ASSERT_FLOAT_EQ(val1, val2, eps) \
    do { \
        if (fabsf((val1) - (val2)) > (eps)) { \
            printf("[FAIL] %s:%d: Assertion failed: %f != %f (eps=%f)\n", __FILE__, __LINE__, (double)(val1), (double)(val2), (double)(eps)); \
            return false; \
        } \
    } while (0)

#define RUN_TEST(test_func) \
    do { \
        printf("[RUN] %s...\n", #test_func); \
        if (test_func()) { \
            printf("[PASS] %s\n", #test_func); \
            passed++; \
        } else { \
            failed++; \
        } \
    } while (0)

/* Test 1: Vector L2 Norm */
static bool test_vector_l2_norm(void) {
    float32_t vec[3] = {3.0f, 4.0f, 0.0f};
    float32_t norm = tensor_vec_l2_norm(vec, 3U);
    ASSERT_FLOAT_EQ(norm, 5.0f, 1e-5f);

    /* Bounds check and contract validation */
    float32_t err_norm = tensor_vec_l2_norm(NULL, 3U);
    ASSERT_TRUE(err_norm < 0.0f);

    return true;
}

/* Test 2: Matrix-Vector Product */
static bool test_matrix_vector_mult(void) {
    /* 2x3 matrix */
    float32_t A[6] = {
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f
    };
    float32_t x[3] = {1.0f, -1.0f, 2.0f};
    float32_t y[2] = {0.0f, 0.0f};

    bool success = tensor_mat_vec_mult(y, A, x, 2U, 3U);
    ASSERT_TRUE(success);
    /* y[0] = 1*1 + 2*-1 + 3*2 = 5 */
    /* y[1] = 4*1 + 5*-1 + 6*2 = 11 */
    ASSERT_FLOAT_EQ(y[0], 5.0f, 1e-5f);
    ASSERT_FLOAT_EQ(y[1], 11.0f, 1e-5f);

    /* Null pointer safety check */
    success = tensor_mat_vec_mult(NULL, A, x, 2U, 3U);
    ASSERT_TRUE(!success);

    return true;
}

/* Test 3: Convex Projection Safety */
static bool test_convex_projection(void) {
    /* 2x2 matrix with Frobenius norm exceeding max limit */
    float32_t W[4] = {
        3.0f, 0.0f,
        0.0f, 4.0f
    }; /* Frobenius norm = sqrt(9 + 16) = 5.0 */

    /* Project to max norm of 2.5 */
    project_weight_matrix(W, 2U, 2U, 2.5f);

    /* Norm should now be scaled to exactly 2.5 */
    float32_t norm = tensor_vec_l2_norm(W, 4U);
    ASSERT_FLOAT_EQ(norm, 2.5f, 1e-5f);

    /* Elements should be scaled by 0.5 */
    ASSERT_FLOAT_EQ(W[0], 1.5f, 1e-5f);
    ASSERT_FLOAT_EQ(W[3], 2.0f, 1e-5f);

    return true;
}

/* Test 4: Decoder State Transitions & SNN LIF Spiking */
static bool test_decoder_lif_and_transitions(void) {
    DecoderState state;
    DecoderConfig config;

    config.snn_decay = 0.5f;
    config.snn_threshold = 1.0f;
    config.dt = 0.001f;
    config.stability_c = 1.0f;

    /* Zero out transitions */
    memset(config.A, 0, sizeof(config.A));
    memset(config.B, 0, sizeof(config.B));
    memset(config.W_snn, 0, sizeof(config.W_snn));

    /* Connect SNN 0 to sensor 0 with weight 2.0 */
    config.W_snn[0][0] = 2.0f;
    /* Connect trajectory element 1 to SNN 0 with weight 0.5 */
    config.B[1][0] = 0.5f;

    core_neural_init(&state, &config);

    float32_t raw_lfp[OBS_DIM];
    memset(raw_lfp, 0, sizeof(raw_lfp));
    raw_lfp[0] = 1.0f; /* LFP stimulus of 1.0 on channel 0 */

    /* Decode Step: LIF input = 2.0 * 1.0 = 2.0.
     * New potential v[0] = 0*0.5 + 2.0 = 2.0.
     * 2.0 >= threshold (1.0), so spike occurs (spikes[0]=1), and potential resets to 0. */
    core_neural_decode_step(&state, &config, raw_lfp);

    ASSERT_TRUE(state.snn.spikes[0] == 1U);
    ASSERT_FLOAT_EQ(state.snn.v[0], 0.0f, 1e-5f);

    /* Trajectory state update should receive B * spikes:
     * state.traj.x[1] += 0.5 * 1.0 = 0.5 */
    ASSERT_FLOAT_EQ(state.traj.x[1], 0.5f, 1e-5f);

    return true;
}

/* Test 5: Exact Bit-Level Deterministic Reproducibility */
static bool test_bit_level_determinism(void) {
    /* Running exact identical mathematical operations sequentially must produce
     * identical binary representation down to the exact bit-level */
    float32_t vec_a[4] = {1.234567f, -9.876543f, 0.000123f, 456.789123f};
    float32_t vec_b[4] = {1.234567f, -9.876543f, 0.000123f, 456.789123f};

    float32_t norm_a = tensor_vec_l2_norm(vec_a, 4U);
    float32_t norm_b = tensor_vec_l2_norm(vec_b, 4U);

    uint32_t bit_pattern_a;
    uint32_t bit_pattern_b;

    /* Copy binary representation directly without float casting conversions */
    memcpy(&bit_pattern_a, &norm_a, sizeof(float32_t));
    memcpy(&bit_pattern_b, &norm_b, sizeof(float32_t));

    ASSERT_TRUE(bit_pattern_a == bit_pattern_b);

    return true;
}

/* Test 6: Safe Boundary Compliance */
static bool test_safety_boundary_compliance(void) {
    SystemHealthStatus status;

    /* Healthy system */
    status.current_latency_ms = 3.5f;
    status.packet_loss_rate = 0.0005f;
    for (uint32_t i = 0U; i < OBS_DIM; ++i) {
        status.channel_impedance[i] = 40.0f;
    }

    ASSERT_TRUE(verify_system_boundaries(&status) == true);

    /* Fault 1: High latency */
    status.current_latency_ms = 5.5f;
    ASSERT_TRUE(verify_system_boundaries(&status) == false);
    status.current_latency_ms = 3.5f;

    /* Fault 2: High packet loss */
    status.packet_loss_rate = 0.002f;
    ASSERT_TRUE(verify_system_boundaries(&status) == false);
    status.packet_loss_rate = 0.0005f;

    /* Fault 3: High impedance on some channel */
    status.channel_impedance[42] = 51.5f;
    ASSERT_TRUE(verify_system_boundaries(&status) == false);

    return true;
}

/* --- RUNNER --- */
int main(void) {
    uint32_t passed = 0U;
    uint32_t failed = 0U;

    printf("====================================================================\n");
    printf("                  CORE_NEURAL UNIT TEST SUITE\n");
    printf("====================================================================\n");

    RUN_TEST(test_vector_l2_norm);
    RUN_TEST(test_matrix_vector_mult);
    RUN_TEST(test_convex_projection);
    RUN_TEST(test_decoder_lif_and_transitions);
    RUN_TEST(test_bit_level_determinism);
    RUN_TEST(test_safety_boundary_compliance);

    printf("--------------------------------------------------------------------\n");
    printf("TEST RESULTS: PASSED: %u | FAILED: %u\n", passed, failed);
    printf("====================================================================\n");

    return (failed > 0U) ? 1 : 0;
}
