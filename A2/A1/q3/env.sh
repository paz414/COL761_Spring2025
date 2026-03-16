#!/bin/bash
set -euo pipefail

# Create a local venv and install minimal deps
python3 -m venv .venv
source .venv/bin/activate

python -m pip install --upgrade pip
python -m pip install numpy
