# Core_Neural: High-Performance, Memory-Safe Neural Compute Engine

[![License: CC0-1.0](https://img.shields.io/badge/License-CC0_1.0-lightgrey.svg)](https://creativecommons.org/publicdomain/zero/1.0/)
[![Rust CI/CD](https://github.com/DMerritt-Nexorian/Core_Neural/actions/workflows/ci.yml/badge.svg)](https://github.com/DMerritt-Nexorian/Core_Neural/actions/workflows/ci.yml)

`core_neural` is a bare-metal ready, high-performance, and mathematically verified neural state-space decoding and neuromorphic computing library written in pure **Rust**. It is designed to run in safety-critical neuro-prosthetic applications with strict real-time deadlines.

The library achieves absolute **Technology Readiness Level 9 (TRL-9)** operational readiness by enforcing:
* **Zero Runtime Allocation:** All states and configurations are stack-allocated with fixed dimensions, completely eliminating runtime garbage collection pauses and memory fragmentation.
* **Deterministic O(1) Execution Complexity:** Zero branch-based timing leaks, ensuring exact constant-time loop performance under peak loads.
* **Strict Mathematical Invariants:** Hardcoded discretized contractive stability boundaries and convex Frobenius norm projection operators prevent trajectory divergence under adversarial inputs.

---

## 1. Directory Structure

```text
.
├── .github/
│   └── workflows/
│       └── ci.yml               # Automated CI/CD (linting, checks, tests, bench)
├── benches/
│   └── neural_bench.rs         # High-precision micro-benchmarking suite
├── src/
│   ├── decoder.rs              # State-space kinematic trajectory decoding
│   ├── error.rs                # Custom type-safe error handling
│   ├── lib.rs                  # Library entry point & API re-exports
│   ├── snn.rs                  # Neuromorphic LIF spiking neural network
│   └── stability.rs            # Contractive stability & weight projections
├── Cargo.toml                  # Cargo configuration with release profiles
├── MATH_SPEC.md                # Linear algebra & dynamical systems specification
├── STREAM_INGRESS.md           # Zero-copy ingress & buffer overflow prevention guide
├── TECHNICAL_SPEC.md           # Floating-point safety & thread-safety invariants
└── CONTRIBUTING.md             # High-assurance development guidelines
```

---

## 2. API Quickstart

### Ingesting raw signals and decoding trajectory steps:
```rust
use core_neural::{decode_step, DecoderConfig, DecoderState, SnnConfig, OBS_DIM, SNN_NEURONS, STATE_DIM};

// 1. Initialize stable state-space transition weights
let a = [[0.9f32; STATE_DIM]; STATE_DIM];
let b = [[0.01f32; SNN_NEURONS]; STATE_DIM];
let snn_weights = [[0.05f32; OBS_DIM]; SNN_NEURONS];

let config = DecoderConfig {
    a,
    b,
    snn_config: SnnConfig {
        decay: 0.8,
        threshold: 1.0,
        weights: snn_weights,
    },
    dt: 0.001,
    stability_c: 10.0,
};

// 2. Initialize exclusive mutable state
let mut state = DecoderState::default();

// 3. Process telemetry frame in real-time execution loop
let raw_lfp = [1.2f32; OBS_DIM]; // 64-channel array
decode_step(&mut state, &config, &raw_lfp).unwrap();

println!("Updated trajectory state: {:?}", state.trajectory);
```

---

## 3. High-Assurance Verification Commands

Enforce extreme compliance by executing our unified test, style, and performance quality gates locally:

* **Strict Formatting & Style Verification:**
  ```bash
  cargo fmt --all -- --check
  ```
* **Strict Compiler Linting (Zero Warnings Allowed):**
  ```bash
  cargo clippy --all-targets -- -D warnings
  ```
* **Exhaustive Unit Tests:**
  ```bash
  cargo test --all
  ```
* **Micro-Benchmarking Execution:**
  ```bash
  cargo bench
  ```
