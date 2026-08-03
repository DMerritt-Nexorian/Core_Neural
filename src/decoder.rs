use crate::error::NeuralEngineError;
use crate::snn::{OBS_DIM, SNN_NEURONS, SnnConfig, SnnState, snn_update_step};

/// Dimension of the kinematic trajectory state vector (x_pos, y_pos, x_vel, y_vel).
pub const STATE_DIM: usize = 4;

/// Complete BCI decoder state comprising the neuromorphic SNN and continuous trajectory states.
#[derive(Debug, Clone, Copy)]
pub struct DecoderState {
    /// Active on-chip Spiking Neural Network state.
    pub snn_state: SnnState,
    /// Kinematic state vector representingdecoded kinematic trajectory elements.
    pub trajectory: [f32; STATE_DIM],
}

impl Default for DecoderState {
    fn default() -> Self {
        Self {
            snn_state: SnnState::default(),
            trajectory: [0.0; STATE_DIM],
        }
    }
}

/// Parameters and state-space transitions of the BCI Decoder.
#[derive(Debug, Clone)]
pub struct DecoderConfig {
    /// Trajectory state transition matrix (STATE_DIM x STATE_DIM).
    pub a: [[f32; STATE_DIM]; STATE_DIM],
    /// State control input mapping matrix from SNN spikes (STATE_DIM x SNN_NEURONS).
    pub b: [[f32; SNN_NEURONS]; STATE_DIM],
    /// On-chip Spiking Neural Network configurations.
    pub snn_config: SnnConfig,
    /// Discrete execution timestep size (seconds).
    pub dt: f32,
    /// Contractive stability decay factor c > 0.
    pub stability_c: f32,
}

impl DecoderConfig {
    /// Validates all parameters to ensure mathematical and operational boundaries.
    pub fn validate(&self) -> Result<(), NeuralEngineError> {
        self.snn_config.validate()?;
        if self.dt <= 0.0 {
            return Err(NeuralEngineError::InvalidConfiguration);
        }
        if self.stability_c <= 0.0 {
            return Err(NeuralEngineError::InvalidConfiguration);
        }
        Ok(())
    }
}

/// Perform a single discrete real-time decoding update step.
#[allow(clippy::needless_range_loop)]
pub fn decode_step(
    state: &mut DecoderState,
    config: &DecoderConfig,
    raw_lfp: &[f32; OBS_DIM],
) -> Result<(), NeuralEngineError> {
    config.validate()?;

    // STAGE 1: Update on-chip Spiking Neural Network State
    snn_update_step(&mut state.snn_state, &config.snn_config, raw_lfp)?;

    // STAGE 2: Compute Kinematic Trajectory State-Space Update
    // X_{t+1} = A * X_t + B * S_t
    let mut term_a = [0.0f32; STATE_DIM];
    let mut term_b = [0.0f32; STATE_DIM];

    // Compute A * X_t
    for i in 0..STATE_DIM {
        let mut sum = 0.0;
        for j in 0..STATE_DIM {
            sum += config.a[i][j] * state.trajectory[j];
        }
        term_a[i] = sum;
    }

    // Compute B * S_t
    for i in 0..STATE_DIM {
        let mut sum = 0.0;
        for j in 0..SNN_NEURONS {
            sum += config.b[i][j] * (state.snn_state.spikes[j] as f32);
        }
        term_b[i] = sum;
    }

    // Update trajectory: X_{t+1} = term_a + term_b
    for i in 0..STATE_DIM {
        state.trajectory[i] = term_a[i] + term_b[i];
    }

    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    #[allow(clippy::needless_range_loop)]
    fn test_decoder_trajectory_transition() {
        let mut state = DecoderState {
            trajectory: [1.0, 2.0, 3.0, 4.0],
            ..Default::default()
        };

        let mut a = [[0.0f32; STATE_DIM]; STATE_DIM];
        for i in 0..STATE_DIM {
            a[i][i] = 0.9; // Stable state transition diagonal
        }

        let mut b = [[0.0f32; SNN_NEURONS]; STATE_DIM];
        b[0][0] = 0.5; // Spike 0 maps 0.5 into dimension 0

        let mut snn_weights = [[0.0f32; OBS_DIM]; SNN_NEURONS];
        snn_weights[0][0] = 2.0;

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

        let mut raw_lfp = [0.0f32; OBS_DIM];
        raw_lfp[0] = 1.0;

        // Run step
        // SNN spikes[0] will be 1.
        // trajectory[0] = 0.9 * 1.0 + 0.5 * 1.0 = 1.4
        // trajectory[1] = 0.9 * 2.0 + 0.0 = 1.8
        decode_step(&mut state, &config, &raw_lfp).unwrap();

        assert_eq!(state.trajectory[0], 1.4);
        assert_eq!(state.trajectory[1], 1.8);
    }
}
