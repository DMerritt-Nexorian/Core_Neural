from src.decoding_validator import verify_neural_decoding_latency, verify_tissue_impedance

def test_latency_bounds():
    assert verify_neural_decoding_latency(2.1, 0.0001) is True
    assert verify_neural_decoding_latency(6.0, 0.0001) is False

def test_impedance_bounds():
    assert verify_tissue_impedance(45.0) is True
    assert verify_tissue_impedance(55.0) is False
