use crate::error::NeuralEngineError;

/// Calculate the Euclidean (L2) norm of a vector slice.
pub fn l2_norm(vector: &[f32]) -> f32 {
    let mut sum_squares = 0.0;
    for &val in vector {
        sum_squares += val * val;
    }
    sum_squares.sqrt()
}

/// Verifies discretized contractive stability:
/// ||next_x1 - next_x2|| <= (1 - c * dt) * ||x1 - x2||
#[allow(clippy::needless_range_loop)]
pub fn verify_contractive_stability(
    x1: &[f32],
    x2: &[f32],
    next_x1: &[f32],
    next_x2: &[f32],
    dt: f32,
    c: f32,
) -> Result<bool, NeuralEngineError> {
    if x1.len() != x2.len() || x1.len() != next_x1.len() || x1.len() != next_x2.len() {
        return Err(NeuralEngineError::DimensionMismatch);
    }
    if dt <= 0.0 || c <= 0.0 {
        return Err(NeuralEngineError::InvalidConfiguration);
    }

    let len = x1.len();
    let mut sum_diff_t = 0.0;
    let mut sum_diff_t_next = 0.0;

    for i in 0..len {
        let d_t = x1[i] - x2[i];
        let d_t_next = next_x1[i] - next_x2[i];
        sum_diff_t += d_t * d_t;
        sum_diff_t_next += d_t_next * d_t_next;
    }

    let norm_t = sum_diff_t.sqrt();
    let norm_t_next = sum_diff_t_next.sqrt();

    let decay_factor = 1.0 - (c * dt);
    let limit = decay_factor * norm_t;

    Ok(norm_t_next <= limit)
}

/// Enforces weight matrix constraints by projecting a matrix slice onto the convex
/// constraint set C, defined by a Frobenius norm limit.
/// W_projected = W * min(1, max_norm / ||W||_Frobenius)
pub fn project_weight_matrix(matrix: &mut [f32], max_norm: f32) -> Result<(), NeuralEngineError> {
    if max_norm <= 0.0 {
        return Err(NeuralEngineError::InvalidConfiguration);
    }

    let frob_norm = l2_norm(matrix);

    if frob_norm > max_norm && frob_norm > 1e-6 {
        let scale = max_norm / frob_norm;
        for val in matrix.iter_mut() {
            *val *= scale;
        }
    }

    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_l2_norm() {
        let vec = [3.0, 4.0, 0.0];
        assert_eq!(l2_norm(&vec), 5.0);
    }

    #[test]
    fn test_weight_projection() {
        let mut matrix = [3.0, 0.0, 0.0, 4.0]; // Frobenius norm = 5.0
        project_weight_matrix(&mut matrix, 2.5).unwrap();
        assert_eq!(l2_norm(&matrix), 2.5);
        assert_eq!(matrix[0], 1.5);
        assert_eq!(matrix[3], 2.0);
    }

    #[test]
    fn test_contractive_stability() {
        let x1 = [1.0, 2.0];
        let x2 = [1.0, 1.0];
        let next_x1 = [1.0, 1.8];
        let next_x2 = [1.0, 1.0];

        // diff_t = [0, 1] (norm_t = 1.0)
        // diff_t_next = [0, 0.8] (norm_t_next = 0.8)
        // decay_factor = 1.0 - 10.0 * 0.01 = 0.9.
        // limit = 0.9 * 1.0 = 0.9.
        // 0.8 <= 0.9 => stable.
        let stable =
            verify_contractive_stability(&x1, &x2, &next_x1, &next_x2, 0.01, 10.0).unwrap();
        assert!(stable);
    }
}
