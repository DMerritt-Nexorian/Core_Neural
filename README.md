# MASTER TECHNICAL SPECIFICATION: CORE_NEURAL
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.21605054.svg)](https://doi.org/10.5281/zenodo.21605054)

## Ultra-Flexible Subdermal Brain-Computer Interface Neural Decoders

[![License: CC0-1.0](https://img.shields.io/badge/License-CC0_1.0-lightgrey.svg)](https://creativecommons.org/publicdomain/zero/1.0/)

---

### PUBLIC DOMAIN DEDICATION (CC0 1.0 UNIVERSAL)

**STATEMENT OF PURPOSE:**
To the extent possible under law, the author(s) and contributor(s) of this Master Technical Specification have dedicated all copyright and related or neighboring rights to this document worldwide. You may copy, modify, distribute, perform, and manufacture based on this work, even for commercial purposes, all without asking permission or paying royalties. This document is provided as a non-binding technical specification and does not constitute medical advice.

* **Official Reference:** [Creative Commons CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/)

---

## EXECUTIVE SUMMARY

This repository contains the CORE_NEURAL Master Technical Specification alongside implementation utilities for validating key performance and safety boundaries for real-time neural decoding systems. The primary artifact is the specification PDF and supporting Python utilities for validation and testing.

---

## What's changed (README update)

- A lightweight runtime validator script has been added at `src/decoding_validator.py` to programmatically check key operational boundaries described in the spec (latency, packet loss, tissue impedance).
- This README has been updated to document how to use the validator, its public API, and quickstart instructions.

---

## Quickstart: run the validator

Requirements:
- Python 3.8+ (recommended)

From the repository root you can run the validator directly:

```bash
python src/decoding_validator.py
```

This will execute a simple self-test that asserts the example values meet the specification limits; on success it prints:

[SUCCESS] CORE_NEURAL validation checks passed.

Alternatively, import the validator functions into your own tests or CI checks:

```python
from src.decoding_validator import verify_neural_decoding_latency, verify_tissue_impedance

# Example checks
print(verify_neural_decoding_latency(2.1, 0.0001))  # True
print(verify_tissue_impedance(35.0))                 # True
```

---

## API: src/decoding_validator.py

- verify_neural_decoding_latency(latency_ms: float, packet_loss_rate: float) -> bool
  - Returns True if latency_ms <= 5.0 ms and packet_loss_rate <= 0.001 (0.1%).

- verify_tissue_impedance(impedance_kohm: float) -> bool
  - Returns True if impedance_kohm < 50.0 (kΩ).

These functions are intentionally small and dependency-free so they can be embedded in CI pipelines or hardware-in-the-loop tests.

---

## Repository contents

- CORE_NEURAL_Master_Technical_Spec_CC0.pdf⁠.pdf — Full technical specification (Master Spec)
- README.md — This document (updated)
- LICENSE — CC0 1.0 Universal public domain dedication
- src/ — Reference implementations and validation utilities (validator currently)
- tests/ — Placeholder for test suites

---

## Notes and disclaimers

This repository and the specification describe research-oriented hardware and software ideas. They are not medical advice and are not a substitute for regulatory compliant development, clinical trials, or professional medical judgment. Implementations targeting human use must follow applicable laws, medical device regulations, and institutional review.

---

DOCUMENT END & STATUS

* Author / Entity: Dennis W. Merritt / Nexorian Corporation
* Contact: NexorianLabs@icloud.com
* Target Application: Paralyzed patient mobility restoration, motor-cognitive rehabilitation, and neuro-prosthetic control.
* License: Public Domain Contribution under Creative Commons CC0 1.0 Universal.
