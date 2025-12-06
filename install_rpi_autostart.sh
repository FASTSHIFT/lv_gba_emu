#!/bin/bash

# Check if running as root
if [ "$EUID" -ne 0 ]; then
  echo "Please run as root"
  exit
fi

# Get absolute path to current directory
PROJECT_ROOT=$(pwd)
BIN_NAME="gba_emu"
SERVICE_NAME="lv_gba_emu.service"

# Detect real user (who ran sudo)
if [ -n "$SUDO_USER" ]; then
    REAL_USER=$SUDO_USER
    REAL_UID=$(id -u $SUDO_USER)
else
    REAL_USER=$(whoami)
    REAL_UID=$(id -u)
fi

echo "Installing for user: $REAL_USER (UID: $REAL_UID)"

if [ "$1" == "uninstall" ]; then
    echo "Uninstalling $SERVICE_NAME..."
    systemctl stop $SERVICE_NAME || true
    systemctl disable $SERVICE_NAME || true
    rm -f /etc/systemd/system/$SERVICE_NAME
    systemctl daemon-reload
    echo "Uninstallation complete."
    exit 0
fi

# Ensure we are in the project root
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found. Please run this script from the project root."
    exit 1
fi

if [ -f "build/$BIN_NAME" ]; then
    echo "Binary found at build/$BIN_NAME. Skipping build."
else
    echo "Building project..."
    # Clean and build
    rm -rf build
    mkdir build
    cd build
    cmake ..
    make -j$(nproc)
    cd ..

    if [ ! -f "build/$BIN_NAME" ]; then
        echo "Build failed!"
        exit 1
    fi
fi

echo "Configuring service to run from: $PROJECT_ROOT"

# Create systemd service
echo "Creating systemd service..."
cat > /etc/systemd/system/$SERVICE_NAME <<EOF
[Unit]
Description=LVGL GBA Emulator
After=network.target systemd-user-sessions.service plymouth-quit-wait.service

[Service]
Type=simple
ExecStart=$PROJECT_ROOT/build/$BIN_NAME -d rom
WorkingDirectory=$PROJECT_ROOT
Restart=no
User=$REAL_USER
Group=audio
SupplementaryGroups=gpio input video render
Environment=TERM=linux
Environment=XDG_RUNTIME_DIR=/run/user/$REAL_UID

[Install]
WantedBy=multi-user.target
EOF

echo "Enabling service..."
systemctl daemon-reload
systemctl enable $SERVICE_NAME
systemctl start $SERVICE_NAME

echo "Installation complete! The emulator should now start on boot."
echo "Service file created at: /etc/systemd/system/$SERVICE_NAME"
echo "You can check the status with: systemctl status $SERVICE_NAME"
echo "To uninstall, run: $0 uninstall"
