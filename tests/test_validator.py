import pytest
from src.decoding_validator import verify_neural_decoding_latency, verify_tissue_impedance


def test_latency_pass():
    assert verify_neural_decoding_latency(2.1, 0.0001) is True


def test_latency_fail():
    # latency exceeds 5.0 ms
    assert verify_neural_decoding_latency(6.0, 0.0) is False


def test_packet_loss_fail():
    # packet loss exceeds 0.001 (0.1%)
    assert verify_neural_decoding_latency(1.0, 0.01) is False


def test_impedance_pass():
    assert verify_tissue_impedance(35.0) is True


def test_impedance_fail():
    assert verify_tissue_impedance(60.0) is False
