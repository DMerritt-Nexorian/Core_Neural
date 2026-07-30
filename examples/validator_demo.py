"""
Simple example demonstrating passing and failing conditions for the CORE_NEURAL validator.
Run: python examples/validator_demo.py
"""
from src.decoding_validator import verify_neural_decoding_latency, verify_tissue_impedance


def show_case(latency, loss, impedance):
    print(f"Testing latency={latency} ms, packet_loss_rate={loss}, impedance={impedance} kΩ")
    print("  -> Latency OK:", verify_neural_decoding_latency(latency, loss))
    print("  -> Impedance OK:", verify_tissue_impedance(impedance))
    print("-" * 48)


if __name__ == "__main__":
    print("PASSING CASE:")
    show_case(latency=2.1, loss=0.0001, impedance=35.0)

    print("\nFAILING CASES:")
    show_case(latency=6.0, loss=0.0000, impedance=35.0)   # latency too high
    show_case(latency=1.0, loss=0.01, impedance=35.0)     # packet loss too high
    show_case(latency=2.0, loss=0.0001, impedance=60.0)   # impedance too high
