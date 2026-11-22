#!/bin/bash
# Installs the user-space part of the keyboard backlight (needs GalaxyBookKbd.kext + SSDT-GBKB in OpenCore).
set -e
cd "$(dirname "$0")"
sudo mkdir -p /usr/local/bin
sudo install -m 755 kbdlight gbkbd-init /usr/local/bin/
sudo install -m 644 com.vik.gbkbd.plist /Library/LaunchDaemons/
sudo launchctl bootstrap system /Library/LaunchDaemons/com.vik.gbkbd.plist 2>/dev/null || true
echo "Installed. Log: /var/log/gbkbd.log. Manual control: kbdlight 0..3 (level), kbdlight (status)."
