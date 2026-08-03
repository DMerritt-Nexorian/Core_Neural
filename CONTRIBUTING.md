# Core_Neural: Contributing Guidelines

Thank you for contributing to the `core_neural` high-assurance compute engine. To maintain our absolute Technology Readiness Level 9 (TRL-9) standards, all code changes must strictly adhere to the following guidelines.

---

## 1. Zero-Allocation Rule

* All code executed inside the core runtime or event loop must be completely free of dynamic memory allocations.
* **Prohibited:** Usage of `Box`, `Vec` (during runtime steps), `Rc`, `Arc`, `HashMap`, or any other allocating structures.
* All arrays must have statically declared compile-time bounds.

---

## 2. Mathematical Rigor & Numerical Stability

* Any new neural decoding, filter, or signal processing algorithm must include a formal mathematical specification in `MATH_SPEC.md`.
* Ensure proper floating-point sanitization:
  - Check that all parameter initializations are finite (`f32::is_finite()`).
  - Projection operators must be implemented if the state transition weights are susceptible to numerical divergence.

---

## 3. Strict Testing & Quality Gates

Before submitting any Pull Request, ensure that:
1. **Formatting & Lints:**
   Code must compile cleanly without any warnings.
   ```bash
   cargo fmt --all -- --check
   cargo clippy --all-targets -- -D warnings
   ```
2. **Deterministic Tests:**
   Add unit tests validating exact output states on constant input data.
   ```bash
   cargo test --all
   ```
3. **Micro-Benchmarking:**
   Run the benchmark suite to verify that changes do not introduce performance regressions or timing leaks.
   ```bash
   cargo bench
   ```
