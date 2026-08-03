#!/usr/bin/env bash
# ====================================================================
#               CORE_NEURAL RUST AUDIT & COMPLIANCE GATE
# ====================================================================
# This script executes the automated daily compliance check for the
# high-assurance Rust neural compute engine. It runs formattings, lints,
# unit tests, and compiles benchmarks to ensure zero regression.
# It outputs a standard JSON audit report under `audit_logs/`.

set -e

echo "===================================================================="
echo "          STARTING CORE_NEURAL PERIODIC COMPLIANCE AUDIT"
echo "===================================================================="

# Create audit logs directory
mkdir -p audit_logs

TIMESTAMP=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
STATUS="SUCCESS"
FORMAT_STATUS="SUCCESS"
LINT_STATUS="SUCCESS"
TEST_STATUS="SUCCESS"
BENCH_STATUS="SUCCESS"

# 1. Format Check
echo "[AUDIT] Checking code formatting..."
if ! cargo fmt --all -- --check; then
    echo "[FAIL] Formatting validation failed!"
    FORMAT_STATUS="FAILED"
    STATUS="FAILED"
fi

# 2. Clippy Lints
echo "[AUDIT] Running strict clippy check (zero warnings allowed)..."
if ! cargo clippy --all-targets -- -D warnings; then
    echo "[FAIL] Clippy verification failed!"
    LINT_STATUS="FAILED"
    STATUS="FAILED"
fi

# 3. Unit Tests
echo "[AUDIT] Executing unit tests..."
if ! cargo test --all; then
    echo "[FAIL] Unit tests failed!"
    TEST_STATUS="FAILED"
    STATUS="FAILED"
fi

# 4. Benchmarks Check
echo "[AUDIT] Verifying benchmarks compilation..."
if ! cargo check --benches; then
    echo "[FAIL] Benchmark compilation failed!"
    BENCH_STATUS="FAILED"
    STATUS="FAILED"
fi

# Write the compliance audit report in JSON
CAT_REPORT="audit_logs/audit_report.json"
cat <<EOF > "$CAT_REPORT"
{
  "timestamp": "$TIMESTAMP",
  "engine": "core_neural_rust",
  "compliance_status": "$STATUS",
  "checks": {
    "formatting": "$FORMAT_STATUS",
    "clippy_lints": "$LINT_STATUS",
    "unit_tests": "$TEST_STATUS",
    "benchmarks": "$BENCH_STATUS"
  }
}
EOF

echo "===================================================================="
echo "          AUDIT COMPLETE: STATUS IS $STATUS"
echo "          Report archived at $CAT_REPORT"
echo "===================================================================="

if [ "$STATUS" = "FAILED" ]; then
    exit 1
fi
