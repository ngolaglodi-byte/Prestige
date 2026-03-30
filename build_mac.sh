#!/bin/bash
set -e

echo "=== Prestige AI — Build macOS ==="

# Detect architecture
ARCH=$(uname -m)
echo "Architecture: $ARCH"

# Find Qt6
if [ -d "/opt/homebrew/opt/qt" ]; then
    QT_PATH="/opt/homebrew"
elif [ -d "/opt/homebrew/opt/qt@6" ]; then
    QT_PATH="/opt/homebrew/opt/qt@6"
elif [ -d "/usr/local/opt/qt@6" ]; then
    QT_PATH="/usr/local/opt/qt@6"
elif [ -d "/usr/local/opt/qt" ]; then
    QT_PATH="/usr/local"
else
    echo "ERREUR: Qt6 non trouvé. Lance: brew install qt@6"
    exit 1
fi
echo "Qt path: $QT_PATH"

# Check for Ninja
GENERATOR=""
if command -v ninja &> /dev/null; then
    GENERATOR="-G Ninja"
    echo "Generator: Ninja"
else
    echo "Generator: Make (install ninja for faster builds: brew install ninja)"
fi

# Clean build
rm -rf build
mkdir build

# Configure
echo ""
echo "=== Configuration CMake ==="
cmake -B build $GENERATOR \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$QT_PATH" \
  -DPRESTIGE_ENABLE_NDI=OFF \
  -DPRESTIGE_ENABLE_DECKLINK=OFF \
  -DPRESTIGE_ENABLE_AJA=OFF \
  -DPRESTIGE_ENABLE_MAGEWELL=OFF \
  -DCMAKE_OSX_ARCHITECTURES=$ARCH \
  -Wno-dev

echo ""
echo "=== Compilation ==="
cmake --build build -j$(sysctl -n hw.logicalcpu)

echo ""
echo "=== BUILD RÉUSSI ==="
echo "Binaires disponibles :"
echo "  ./build/vision_engine/prestige_vision"
echo "  ./build/control_room/prestige_control"
echo ""
echo "Pour lancer :"
echo "  Terminal 1: cd ai_engine && ./setup_mac.sh && source venv/bin/activate && python main.py --dev"
echo "  Terminal 2: ./build/vision_engine/prestige_vision"
echo "  Terminal 3: ./build/control_room/prestige_control"
