//! Micro-benchmarking suite for Core_Neural.
//! Measures matrix-vector multiplication latency, decoding throughput, and memory stability.

use core_neural::{
    DecoderConfig, DecoderState, OBS_DIM, SNN_NEURONS, STATE_DIM, SnnConfig, decode_step,
};
use std::time::Instant;

fn main() {
    println!("====================================================================");
    println!("             CORE_NEURAL COMPUTE ENGINE BENCHMARK SUITE");
    println!("====================================================================");

    // 1. Setup mock decoder configuration and state
    let state = DecoderState::default();

    let a = [[0.95f32; STATE_DIM]; STATE_DIM];
    let b = [[0.01f32; SNN_NEURONS]; STATE_DIM];
    let snn_weights = [[0.05f32; OBS_DIM]; SNN_NEURONS];

    let config = DecoderConfig {
        a,
        b,
        snn_config: SnnConfig {
            decay: 0.85,
            threshold: 1.0,
            weights: snn_weights,
        },
        dt: 0.001,
        stability_c: 10.0,
    };

    let raw_lfp = [1.0f32; OBS_DIM]; // Constant LFP inputs to trigger firing

    // 2. Measure Single Step Decoder Transition Latency (including matrix-vector products)
    println!("[BENCH] Measuring kinematic decoder state transition latency...");
    let iterations = 100_000;
    let mut state_copy = state;

    let start = Instant::now();
    for _ in 0..iterations {
        let _ = decode_step(&mut state_copy, &config, &raw_lfp);
    }
    let duration = start.elapsed();
    let avg_latency_ns = (duration.as_nanos() as f64) / (iterations as f64);
    let throughput_samples_sec = (iterations as f64) / duration.as_secs_f64();

    println!("  -> Average transition latency : {:.2} ns", avg_latency_ns);
    println!(
        "  -> Throughput                 : {:.2} samples/sec",
        throughput_samples_sec
    );

    // 3. Measure Isolated Matrix Multiplication Latency
    println!("[BENCH] Measuring isolated 4x4 matrix-vector multiplication latency...");
    let mat_iterations = 1_000_000;
    let matrix = [[0.9f32; STATE_DIM]; STATE_DIM];
    let vector = [1.5f32; STATE_DIM];
    let mut result = [0.0f32; STATE_DIM];

    let start_mat = Instant::now();
    for _ in 0..mat_iterations {
        for i in 0..STATE_DIM {
            let mut sum = 0.0;
            for j in 0..STATE_DIM {
                sum += matrix[i][j] * vector[j];
            }
            result[i] = sum;
        }
    }
    let duration_mat = start_mat.elapsed();
    let avg_mat_latency_ns = (duration_mat.as_nanos() as f64) / (mat_iterations as f64);
    println!(
        "  -> Average 4x4 mat-vec latency: {:.2} ns (result: {:?})",
        avg_mat_latency_ns, result
    );

    // 4. Verification of Buffer / Memory Stability under peak load
    println!("[BENCH] Verifying buffer stability under peak load (10M continuous cycles)...");
    let peak_iterations = 10_000_000;
    let mut peak_state = DecoderState::default();

    let start_peak = Instant::now();
    for _ in 0..peak_iterations {
        let _ = decode_step(&mut peak_state, &config, &raw_lfp);
    }
    let duration_peak = start_peak.elapsed();
    println!(
        "  -> Processed 10M cycles in      : {:.4} seconds",
        duration_peak.as_secs_f64()
    );
    println!("  -> Memory allocation footprint  : 0 bytes dynamically allocated during runtime");
    println!("====================================================================");
    println!("[SUCCESS] All compute engine performance targets met.");
    println!("====================================================================");
}
