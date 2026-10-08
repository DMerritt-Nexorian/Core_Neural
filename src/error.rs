use core::fmt;

/// Type-safe error representations for the Core_Neural compute engine.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum NeuralEngineError {
    /// Dimension mismatch during tensor or matrix operations.
    DimensionMismatch,
    /// Invalid configuration parameters (e.g. non-positive values, decay out of bounds).
    InvalidConfiguration,
    /// An input array buffer or telemetry stream is null or empty.
    EmptyBuffer,
    /// Real-time timing or latency boundary violation.
    TimingViolation,
    /// Stability constraint violated.
    StabilityViolation,
}

impl fmt::Display for NeuralEngineError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::DimensionMismatch => write!(f, "Matrix or vector dimensions do not match"),
            Self::InvalidConfiguration => write!(f, "Configuration parameter is out of safe range"),
            Self::EmptyBuffer => write!(f, "Telemetry frame or signal stream buffer is empty"),
            Self::TimingViolation => write!(f, "Real-time execution latency exceeded bounds"),
            Self::StabilityViolation => {
                write!(f, "Mathematical contractive stability check failed")
            }
        }
    }
}

impl std::error::Error for NeuralEngineError {}
