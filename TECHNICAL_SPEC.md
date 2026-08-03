# Core_Neural: Technical Specification

This document details the high-performance computing (HPC) engineering standards, floating-point numeric invariants, and thread-safety constraints defining the `core_neural` engine.

---

## 1. Floating-Point Numeric Stability

To maintain extreme precision and prevent divergence or denormalization:

### IEEE 754 Precision Checks
* **Single Precision (`f32`):** Highly optimized for vector instructions, neuromorphic hardware simulation, and minimal cache utilization.
* **Denormalized Float Prevention:** During execution, floating-point operations must prevent numbers from decaying into extremely small denormalized states (which cause heavy hardware execution penalties). If any weight or potential decays below `1e-30`, it is rounded down to exactly `0.0`.
* **NaN and Infinity Safety:** Configuration parameters must be checked during init. No matrix or weights can contain `NaN` or `Inf`.
  ```rust
  pub fn check_float(val: f32) -> bool {
      val.is_finite() && !val.is_nan()
  }
  ```

---

## 2. Deterministic Memory Footprint

To satisfy ultra-low-power edge constraints and guarantee predictability:
* **Zero Runtime Allocation:** The core computation engine uses strictly stack-allocated, fixed-size matrices. No heap allocations occur inside the inner execution loop (`decode_step`).
* **Deterministic O(1) Operations:** No branching paths exist that depend on raw input values, avoiding runtime timing leaks or variable timing profiles.

---

## 3. Thread-Safety and Concurrency Invariants

The `core_neural` library is designed to run in highly concurrent multi-threaded real-time environments:

### Sync and Send Compliance
* All core data structures (`SnnState`, `DecoderState`, `DecoderConfig`) implement standard Rust `Send` and `Sync` traits.
* Since states are mutable, thread safety is achieved by enforcing **exclusive mutable borrowing** (`&mut`) inside execution threads, or by utilizing lock-free spin-locks/double-buffering patterns upstream.
* Exclusive ownership prevents race conditions and data hazards by design, guaranteed at compile-time by the Rust borrow checker.
