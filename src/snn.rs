use crate::error::NeuralEngineError;

/// Dimension of input observations (electrode channels).
pub const OBS_DIM: usize = 64;
/// Number of simulated spiking neurons.
pub const SNN_NEURONS: usize = 32;

/// Spiking Neural Network (SNN) membrane potentials and spike outputs.
#[derive(Debug, Clone, Copy)]
pub struct SnnState {
    /// Membrane potentials for each of the LIF neurons.
    pub potentials: [f32; SNN_NEURONS],
    /// Binary spike outputs (0 or 1) generated at the current timestep.
    pub spikes: [u8; SNN_NEURONS],
}

impl Default for SnnState {
    fn default() -> Self {
        Self {
            potentials: [0.0; SNN_NEURONS],
            spikes: [0; SNN_NEURONS],
        }
    }
}

/// Synaptic weight configuration and threshold parameters for LIF neurons.
#[derive(Debug, Clone)]
pub struct SnnConfig {
    /// LIF membrane potential decay rate (0.0 to 1.0).
    pub decay: f32,
    /// Minimum membrane voltage threshold required to trigger an action potential.
    pub threshold: f32,
    /// Synaptic weight matrix matching dimensions (SNN_NEURONS x OBS_DIM).
    pub weights: [[f32; OBS_DIM]; SNN_NEURONS],
}

impl SnnConfig {
    /// Validate configuration invariants to prevent arithmetic drift or division-by-zero.
    pub fn validate(&self) -> Result<(), NeuralEngineError> {
        if self.decay < 0.0 || self.decay > 1.0 {
            return Err(NeuralEngineError::InvalidConfiguration);
        }
        if self.threshold <= 0.0 {
            return Err(NeuralEngineError::InvalidConfiguration);
        }
        Ok(())
    }
}

/// Execute a single step of the event-driven Spiking Neural Network.
#[allow(clippy::needless_range_loop)]
pub fn snn_update_step(
    state: &mut SnnState,
    config: &SnnConfig,
    raw_lfp: &[f32; OBS_DIM],
) -> Result<(), NeuralEngineError> {
    config.validate()?;

    for i in 0..SNN_NEURONS {
        // Accumulate weighted synaptic inputs
        let mut synaptic_input = 0.0;
        for j in 0..OBS_DIM {
            synaptic_input += config.weights[i][j] * raw_lfp[j];
        }

        // Leaky Integrate-and-Fire equation: v_t = v_{t-1} * decay + input
        let new_voltage = (state.potentials[i] * config.decay) + synaptic_input;

        if new_voltage >= config.threshold {
            state.spikes[i] = 1;
            state.potentials[i] = 0.0; // Reset membrane potential to resting state
        } else {
            state.spikes[i] = 0;
            state.potentials[i] = new_voltage;
        }
    }

    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_snn_spike_generation() {
        let mut state = SnnState::default();
        let mut weights = [[0.0f32; OBS_DIM]; SNN_NEURONS];

        // Setup weight of 2.0 from LFP channel 0 to SNN neuron 0
        weights[0][0] = 2.0;

        let config = SnnConfig {
            decay: 0.5,
            threshold: 1.0,
            weights,
        };

        let mut raw_lfp = [0.0f32; OBS_DIM];
        raw_lfp[0] = 1.0; // Stimulus of 1.0 on channel 0

        // Step 1: Pot = 2.0 >= 1.0. Should spike and reset Pot to 0.0.
        snn_update_step(&mut state, &config, &raw_lfp).unwrap();
        assert_eq!(state.spikes[0], 1);
        assert_eq!(state.potentials[0], 0.0);

        // Step 2: Stimulus is 0.0. Pot = 0.0 * 0.5 + 0.0 = 0.0. No spike.
        raw_lfp[0] = 0.0;
        snn_update_step(&mut state, &config, &raw_lfp).unwrap();
        assert_eq!(state.spikes[0], 0);
        assert_eq!(state.potentials[0], 0.0);
    }

    #[test]
    fn test_invalid_decay_error() {
        let mut state = SnnState::default();
        let weights = [[0.0f32; OBS_DIM]; SNN_NEURONS];
        let raw_lfp = [0.0f32; OBS_DIM];

        let config = SnnConfig {
            decay: 1.5, // Invalid decay out of bounds
            threshold: 1.0,
            weights,
        };

        let res = snn_update_step(&mut state, &config, &raw_lfp);
        assert_eq!(res, Err(NeuralEngineError::InvalidConfiguration));
    }
}
