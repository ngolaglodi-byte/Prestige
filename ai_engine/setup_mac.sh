#!/bin/bash
set -e

echo "=== Setup Python AI Engine (macOS) ==="

# Use Homebrew Python if available, otherwise system Python
if command -v python3.12 &> /dev/null; then
    PYTHON=python3.12
elif command -v /opt/homebrew/bin/python3.12 &> /dev/null; then
    PYTHON=/opt/homebrew/bin/python3.12
elif command -v python3 &> /dev/null; then
    PYTHON=python3
else
    echo "ERREUR: Python 3 non trouvé. Lance: brew install python@3.12"
    exit 1
fi

echo "Python: $($PYTHON --version)"

# Create venv
$PYTHON -m venv venv
source venv/bin/activate

pip install --upgrade pip

echo ""
echo "=== Installation des dépendances ==="
pip install -r requirements.txt

# InsightFace can fail on Apple Silicon — OpenCV fallback is available
echo ""
echo "=== Installation InsightFace (optionnel) ==="
pip install insightface || echo "⚠ InsightFace non disponible — le fallback OpenCV Haar cascade sera utilisé"

echo ""
echo "=== Setup terminé ==="
echo "Lance avec: source venv/bin/activate && python main.py --dev"
