//! Core_Neural: High-Performance, Memory-Safe Neural Compute Engine
//! Enforces strict mathematical invariants, deterministic memory profiles, and zero runtime allocation.

pub mod decoder;
pub mod error;
pub mod snn;
pub mod stability;

// Re-export core items for ease of library integration
pub use decoder::{DecoderConfig, DecoderState, STATE_DIM, decode_step};
pub use error::NeuralEngineError;
pub use snn::{OBS_DIM, SNN_NEURONS, SnnConfig, SnnState, snn_update_step};
pub use stability::{l2_norm, project_weight_matrix, verify_contractive_stability};
