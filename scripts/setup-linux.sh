#!/usr/bin/env bash
# Install the build toolchain and Qt 6.4.2 development packages on Ubuntu 24.04.
# Includes Xvfb + Mesa so Qt Quick 3D can render on a headless VM.
set -euo pipefail

SUDO=""
if [[ $EUID -ne 0 ]]; then SUDO="sudo"; fi

$SUDO apt-get update
$SUDO apt-get install -y --no-install-recommends \
    build-essential cmake ninja-build pkg-config git \
    qt6-base-dev qt6-base-dev-tools \
    qt6-declarative-dev qt6-declarative-dev-tools \
    qt6-quick3d-dev qt6-quick3d-dev-tools \
    qt6-shadertools-dev \
    qml6-module-qtquick qml6-module-qtquick-window qml6-module-qtquick-controls \
    qml6-module-qtquick-templates qml6-module-qtquick-layouts \
    qml6-module-quick3d qml6-module-quick3d-helpers \
    qml6-module-qtqml-workerscript \
    libgl1-mesa-dev libegl1-mesa-dev libgl1-mesa-dri mesa-utils \
    xvfb xauth x11-utils imagemagick

echo
echo "Installed Qt version:"
/usr/lib/qt6/bin/qtpaths6 --qt-version 2>/dev/null || qmake6 -query QT_VERSION
