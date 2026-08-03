# Core_Neural: Stream Ingress Specification

This document details the interface mechanisms and safety contracts governing the ingestion of multi-channel neural telemetry streams into the `core_neural` engine without introducing risk of runtime buffer overflows, latency spikes, or memory leaks.

---

## 1. Zero-Copy Sample Ingestion

The `core_neural` library exposes strict, non-allocating, and memory-safe interfaces. To ingest stream data (such as Lab Streaming Layer / LSL chunks, or TCP socket buffers):

1. **Borrowing vs. Copying:**
   The library never takes ownership or copies variable-size buffers during the update tick. Instead, telemetry buffers are passed as standard fixed-size references:
   ```rust
   pub fn decode_step(
       state: &mut DecoderState,
       config: &DecoderConfig,
       raw_lfp: &[f32; 64], // Fixed-size stack reference
   ) -> Result<(), NeuralEngineError>;
   ```
2. **Buffer Alignment:**
   Upstream data acquisition frameworks must align their spatial telemetry arrays exactly to the $64$-channel boundary (`[f32; 64]`) before calling the decoding loop.

---

## 2. Prevention of Buffer Overflows

We mathematically guarantee zero buffer overflow risk at compile-time:
* **Fixed-Size Types:** By utilizing Rust's compile-time array length bounds checks, any attempt to pass slices of arbitrary or variable dimensions fails at compile-time.
* **Input Deserialization Sanitization:**
  If an upstream stream returns a dynamically sized slice `&[f32]`, it must be explicitly converted using safe try-into conversions:
  ```rust
  let raw_buffer: &[f32] = fetch_telemetry_stream();
  let aligned_lfp: &[f32; 64] = raw_buffer
      .try_into()
      .map_err(|_| NeuralEngineError::DimensionMismatch)?;
  ```
  This guarantees that out-of-bounds array access can never trigger undefined behavior, segment faults, or buffer overflows.

---

## 3. Real-Time Telemetry Jitter Jitter Controls

Under closed-loop BCI motor execution, sample ingestion latency must remain strictly bounded ($\le 5.0\text{ ms}$). To achieve this:
1. **Thread-Pinning & Zero Cross-Core Lock Contention:**
   The decoding core operates entirely in a single thread thread-per-core pin model without cross-core synchronization or global heap locks.
2. **Deterministic O(1) Complexity:**
   Every operation inside `decode_step` has a strictly bounded execution time, completely eliminating latency spikes and jitter.
