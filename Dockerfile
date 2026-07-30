# Reproducible environment for running validator and examples
FROM python:3.10-slim

WORKDIR /app

# Copy repo contents
COPY . /app

# Install minimal test runner
RUN pip install --no-cache-dir pytest

# Default command runs the validator self-test
CMD ["python", "src/decoding_validator.py"]
